// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
// ordered_json keeps an object's keys in the order they were added; json sorts them.
using nlohmann::ordered_json;

const std::string file = "todo.json";
const int version = 1;
const std::string_view usage = R"(usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID)";

// A problem with todo.json, which stops the program with status 2.
struct Problem : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Task {
    long long id;
    std::string text;
    bool done;
};

// All of a stream's bytes, or nothing if reading fails.
std::optional<std::string> read_all(std::istream& in) {
    std::string data;
    char buffer[4096];
    // read fills the buffer, or fails at the end with gcount() bytes read.
    while (in.read(buffer, sizeof buffer) || in.gcount() > 0) data.append(buffer, in.gcount());
    // Opening a folder works, and reading it fails, which sets badbit.
    if (in.bad()) return std::nullopt;
    return data;
}

// A task from its JSON. get<long long>() would take true as 1, so each value's kind is
// checked first; is_number_integer is false for 1.0.
Task task_from_json(const ordered_json& value) {
    bool ok = value.is_object() && value.size() == 3 && value.contains("id") &&
              value.contains("text") && value.contains("done") &&
              value.at("id").is_number_integer() && value.at("id").get<long long>() >= 1 &&
              value.at("text").is_string() && value.at("done").is_boolean();
    if (!ok) throw Problem("not a to-do list");
    return {value.at("id").get<long long>(), value.at("text").get<std::string>(),
            value.at("done").get<bool>()};
}

// The tasks in todo.json, or none if there's no todo.json.
std::vector<Task> read_tasks() {
    std::ifstream in(file, std::ios::binary);
    // The standard says nothing of errno for streams, but on Linux it says why.
    if (!in && errno == ENOENT) return {};
    std::optional<std::string> text = in ? read_all(in) : std::nullopt;
    if (!text) throw Problem(std::strerror(errno));
    // With false for allow_exceptions, parse gives a "discarded" value instead of
    // throwing an exception when the text isn't JSON.
    ordered_json value = ordered_json::parse(*text, nullptr, false);
    if (value.is_discarded()) throw Problem("not valid JSON");
    // A later version of todo may keep anything else in the file, so the version comes
    // first. == on two numbers compares their values, so that 1.0 == 1.
    bool has_version = value.is_object() && value.contains("version");
    if (has_version && value.at("version").is_number() && value.at("version") != version) {
        throw Problem("unknown version " + value.at("version").dump());
    }
    if (!has_version || value.size() != 2 || !value.at("version").is_number_integer() ||
        !value.contains("tasks") || !value.at("tasks").is_array()) {
        throw Problem("not a to-do list");
    }
    std::vector<Task> tasks;
    std::set<long long> ids;
    for (const ordered_json& item : value.at("tasks")) {
        tasks.push_back(task_from_json(item));
        // insert's second is false if the set had the id already.
        if (!ids.insert(tasks.back().id).second) throw Problem("not a to-do list");
    }
    return tasks;
}

// Writes the tasks to todo.json. It writes a new file and renames it over the old one,
// which replaces it in one step, so that a crash leaves the old list or the new one, and
// never half of one.
void write_tasks(const std::vector<Task>& tasks) {
    ordered_json list = {{"version", version}, {"tasks", ordered_json::array()}};
    for (const Task& task : tasks) {
        list["tasks"].push_back({{"id", task.id}, {"text", task.text}, {"done", task.done}});
    }
    std::string text = list.dump(2) + "\n";  // dump(2): two spaces for each level
    // The standard library can't make a file with a fresh name, so it's todo.json.tmp.
    std::string temporary = file + ".tmp";
    std::error_code error;
    {
        std::ofstream out(temporary, std::ios::binary);
        out << text;
        // C++ has no fsync: an ofstream doesn't give the file descriptor it needs.
        out.close();
        if (!out) error = std::error_code(errno, std::generic_category());
    }
    if (!error) fs::rename(temporary, file, error);  // with an error_code, it doesn't throw
    if (error) {
        std::error_code ignored;
        fs::remove(temporary, ignored);
        throw Problem(error.message());
    }
}

// Carries out a command whose arguments have been checked, and returns the status.
int run(const std::string& command, const std::vector<std::string>& args) {
    std::vector<Task> tasks = read_tasks();
    if (command == "add") {
        long long id = 1;
        for (const Task& task : tasks) id = std::max(id, task.id + 1);
        std::string text;
        for (const std::string& arg : args) text += (text.empty() ? "" : " ") + arg;
        tasks.push_back({id, text, false});
        write_tasks(tasks);
        std::cout << "added " << id << ": " << text << '\n';
        return 0;
    }
    if (command == "list") {
        for (const Task& task : tasks) {
            std::cout << task.id << " [" << (task.done ? 'x' : ' ') << "] " << task.text << '\n';
        }
        return 0;
    }
    // from_chars fails if the ID is too big for a long long, which no task's id can be.
    long long id = 0;
    const char* digits = args[0].c_str();
    bool fits = std::from_chars(digits, digits + args[0].size(), id).ec == std::errc();
    auto found = std::find_if(tasks.begin(), tasks.end(),
                              [&](const Task& task) { return fits && task.id == id; });
    if (found == tasks.end()) {
        std::cerr << "todo: no task " << args[0] << '\n';
        return 1;
    }
    Task task = *found;
    if (command == "done") {
        found->done = true;
    } else {
        tasks.erase(found);
    }
    write_tasks(tasks);
    std::cout << (command == "done" ? "done " : "removed ") << task.id << ": " << task.text
              << '\n';
    return 0;
}

int main(int argc, char* argv[]) {
    std::string command = argc > 1 ? argv[1] : "";
    std::vector<std::string> args(argv + std::min(argc, 2), argv + argc);
    bool is_id = args.size() == 1 && !args[0].empty() &&
                 std::all_of(args[0].begin(), args[0].end(),
                             [](char c) { return c >= '0' && c <= '9'; });
    // The command line is checked before todo.json is read.
    bool ok = (command == "add" && !args.empty()) || (command == "list" && args.empty()) ||
              ((command == "done" || command == "remove") && is_id);
    if (!ok) {
        std::cerr << usage << '\n';
        return 2;
    }
    try {
        return run(command, args);
    } catch (const Problem& problem) {
        std::cerr << "todo: " << file << ": " << problem.what() << '\n';
        return 2;
    }
}
