#include <deque>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

struct Print {
    std::string text;
};
struct Sleep {
    long duration;
};
struct Wait {
    size_t task;  // the number of the task to wait for
};
using Step = std::variant<Print, Sleep, Wait>;

struct Task {
    std::string name;
    std::vector<Step> steps;
    size_t next = 0;  // the index of the step to run when it runs again
    bool done = false;
    size_t waiting_for = 0;
    std::vector<size_t> waiters;  // in the order they began waiting
};

std::string trim(const std::string& text) {
    size_t start = text.find_first_not_of(' ');
    if (start == std::string::npos) return "";
    return text.substr(start, text.find_last_not_of(' ') - start + 1);
}

// The first word of a line, and the rest of it.
std::pair<std::string, std::string> split(const std::string& line) {
    std::string trimmed = trim(line);
    size_t space = trimmed.find(' ');
    if (space == std::string::npos) return {trimmed, ""};
    return {trimmed.substr(0, space), trim(trimmed.substr(space))};
}

// Returns the tasks, and prints an error for each line with a problem.
std::vector<Task> parse(const std::vector<std::string>& lines) {
    // A wait can name a task defined further down, so the names come first.
    std::unordered_map<std::string, size_t> numbers;
    for (const auto& line : lines) {
        auto [command, name] = split(line);
        if (command == "task") numbers.try_emplace(name, numbers.size());
    }
    std::vector<Task> tasks;
    for (size_t i = 0; i < lines.size(); i++) {
        auto [command, rest] = split(lines[i]);
        std::string error = "error: line " + std::to_string(i + 1) + ": ";
        if (command.empty()) continue;
        if (command == "task") {
            tasks.emplace_back().name = rest;
        } else if (tasks.empty()) {
            std::cout << error << "a step before any task\n";
        } else if (command == "print") {
            tasks.back().steps.push_back(Print{rest});
        } else if (command == "sleep") {
            if (!rest.empty() && rest.find_first_not_of("0123456789") == std::string::npos) {
                tasks.back().steps.push_back(Sleep{std::stol(rest)});
            } else {
                std::cout << error << "sleep takes a whole number\n";
            }
        } else if (command == "wait") {
            if (auto found = numbers.find(rest); found != numbers.end()) {
                tasks.back().steps.push_back(Wait{found->second});
            } else {
                std::cout << error << "no task named " << rest << "\n";
            }
        } else {
            std::cout << error << "unknown step: " << command << "\n";
        }
    }
    return tasks;
}

class Loop {
public:
    explicit Loop(std::vector<Task> tasks) : tasks_(std::move(tasks)) {
        for (size_t id = 0; id < tasks_.size(); id++) ready_.push_back(id);
    }

    void run() {
        while (true) {
            while (!ready_.empty()) {
                size_t id = ready_.front();
                ready_.pop_front();
                run_until_stopped(id);
            }
            if (sleeping_.empty()) break;
            // Nothing can run now, so the clock jumps to the next wake.
            time_ = std::get<0>(sleeping_.top());
            while (!sleeping_.empty() && std::get<0>(sleeping_.top()) == time_) {
                ready_.push_back(std::get<2>(sleeping_.top()));
                sleeping_.pop();
            }
        }
        std::string stuck;
        for (const auto& task : tasks_) {
            if (task.done) continue;
            stuck += (stuck.empty() ? "" : ", ") + task.name + " waits for " +
                     tasks_[task.waiting_for].name;
        }
        if (stuck.empty()) {
            std::cout << "[" << time_ << "] all done\n";
        } else {
            std::cout << "[" << time_ << "] deadlock: " << stuck << "\n";
        }
    }

private:
    // Runs a task until it sleeps, waits, or ends. Returns from inside the
    // loop when the task stops before its end.
    void run_until_stopped(size_t id) {
        Task& task = tasks_[id];
        while (task.next < task.steps.size()) {
            const Step& step = task.steps[task.next++];
            if (auto print = std::get_if<Print>(&step)) {
                std::cout << "[" << time_ << "] " << task.name << ": " << print->text << "\n";
            } else if (auto sleep = std::get_if<Sleep>(&step)) {
                if (sleep->duration == 0) {
                    ready_.push_back(id);
                } else {
                    sleeping_.emplace(time_ + sleep->duration, sleeps_++, id);
                }
                return;
            } else if (auto wait = std::get_if<Wait>(&step); !tasks_[wait->task].done) {
                task.waiting_for = wait->task;
                tasks_[wait->task].waiters.push_back(id);
                return;
            }  // a wait for a task that has finished carries on
        }
        task.done = true;
        std::cout << "[" << time_ << "] " << task.name << ": done\n";
        for (size_t waiter : task.waiters) ready_.push_back(waiter);
    }

    std::vector<Task> tasks_;
    long time_ = 0;
    std::deque<size_t> ready_;  // the tasks that can run now, in order
    // The sleeping tasks, as (time to wake, order of the sleep, task). A
    // priority_queue's top is its largest item, so std::greater makes it the
    // smallest.
    using Sleeper = std::tuple<long, long, size_t>;
    std::priority_queue<Sleeper, std::vector<Sleeper>, std::greater<>> sleeping_;
    long sleeps_ = 0;
};

int main() {
    std::vector<std::string> lines;
    for (std::string line; std::getline(std::cin, line);) lines.push_back(line);
    Loop(parse(lines)).run();
}
