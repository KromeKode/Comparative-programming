// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Function {
    long long calls = 0, total = 0, self = 0;
    long long running = 0;          // how many of its calls are running now
    long long outermost_start = 0;  // when the outermost of them started
};

struct Call {
    std::string name;
    long long start;
    long long in_callees = 0;  // time spent in the functions it called
};

class TraceError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

bool is_time(const std::string& s) {
    return !s.empty() && s.size() <= 12 &&
           std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; });
}

bool is_name(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](char c) { return c >= '!' && c <= '~'; });
}

std::map<std::string, Function> profile(std::istream& in) {
    std::map<std::string, Function> functions;
    std::vector<Call> stack;
    long long now = 0;
    std::string line;
    for (int number = 1; std::getline(in, line); number++) {
        std::istringstream words(line);
        std::vector<std::string> fields;
        for (std::string field; words >> field;) fields.push_back(field);
        if (fields.empty()) continue;
        const std::string where = "line " + std::to_string(number) + ": ";
        if (fields.size() != 3 || !is_time(fields[0]) || !is_name(fields[2]) ||
            (fields[1] != "enter" && fields[1] != "exit")) {
            throw TraceError(where + "expected TIME enter NAME or TIME exit NAME");
        }
        long long time = std::stoll(fields[0]);
        const std::string& name = fields[2];
        if (time < now) {
            throw TraceError(where + "time goes back from " + std::to_string(now) + " to " +
                             std::to_string(time));
        }
        now = time;

        if (fields[1] == "enter") {
            Function& function = functions[name];
            function.calls++;
            if (function.running == 0) function.outermost_start = time;
            function.running++;
            stack.push_back({name, time});
            continue;
        }

        if (stack.empty()) {
            throw TraceError(where + "exit from " + name + ", but nothing is running");
        }
        if (stack.back().name != name) {
            throw TraceError(where + "exit from " + name + ", but the innermost call is to " +
                             stack.back().name);
        }
        Call call = stack.back();
        stack.pop_back();
        long long duration = time - call.start;
        Function& function = functions[name];
        function.self += duration - call.in_callees;
        function.running--;
        // A recursive call's time is already in the total of the call around it.
        if (function.running == 0) function.total += time - function.outermost_start;
        if (!stack.empty()) stack.back().in_callees += duration;
    }
    if (!stack.empty()) {
        throw TraceError("end of trace: " + stack.back().name + " is still running");
    }
    return functions;
}

int main() {
    std::map<std::string, Function> functions;
    try {
        functions = profile(std::cin);
    } catch (const TraceError& error) {
        std::cout << "error: " << error.what() << '\n';
        return 0;
    }
    if (functions.empty()) std::cout << "no calls\n";
    std::vector<std::pair<std::string, Function>> rows(functions.begin(), functions.end());
    std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        return a.second.self > b.second.self;
    });
    for (const auto& [name, f] : rows) {
        std::cout << name << ' ' << f.calls << ' ' << f.total << ' ' << f.self << '\n';
    }
}
