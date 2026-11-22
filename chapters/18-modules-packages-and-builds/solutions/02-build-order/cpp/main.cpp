// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Module {
    int line;
    std::vector<std::string> imports;  // as written, with repeats dropped
};

std::vector<std::string> split(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> words;
    for (std::string word; in >> word;) words.push_back(word);
    return words;
}

bool is_name(const std::string& text) {
    return !text.empty() && std::all_of(text.begin(), text.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '.' || c == '/' || c == '-';
    });
}

}  // namespace

int main() {
    std::map<std::string, Module> modules;
    std::vector<std::string> defined;  // the names in the order they were defined
    std::vector<std::pair<int, std::string>> queries;
    std::string line;
    for (int number = 1; std::getline(std::cin, line); number++) {
        std::size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::vector<std::string> name = split(line.substr(0, colon));
            std::vector<std::string> imports = split(line.substr(colon + 1));
            if (name.size() != 1 || !is_name(name[0]) ||
                !std::all_of(imports.begin(), imports.end(), is_name)) {
                std::cout << "error: line " << number << ": a module is NAME: IMPORTS\n";
                return 0;
            }
            if (auto earlier = modules.find(name[0]); earlier != modules.end()) {
                std::cout << "error: line " << number << ": " << name[0]
                          << " is already defined on line " << earlier->second.line << '\n';
                return 0;
            }
            Module module{number, {}};
            for (const std::string& import : imports) {
                if (std::find(module.imports.begin(), module.imports.end(), import) ==
                    module.imports.end()) {
                    module.imports.push_back(import);
                }
            }
            modules[name[0]] = module;
            defined.push_back(name[0]);
            continue;
        }
        std::vector<std::string> words = split(line);
        if (words.empty()) continue;
        if (words.size() == 2 && words[0] == "rebuild") {
            queries.emplace_back(number, words[1]);
            continue;
        }
        std::cout << "error: line " << number << ": expected NAME: IMPORTS or rebuild NAME\n";
        return 0;
    }

    std::map<std::string, std::vector<std::string>> users;
    std::map<std::string, std::size_t> waiting;
    for (const std::string& name : defined) {
        const Module& module = modules[name];
        for (const std::string& import : module.imports) {
            if (!modules.contains(import)) {
                std::cout << "error: line " << module.line << ": " << name << " imports "
                          << import << ", which isn't defined\n";
                return 0;
            }
            users[import].push_back(name);
        }
        waiting[name] = module.imports.size();
    }

    // Kahn's algorithm: a module is ready once everything it imports is built, and
    // of the ready modules, the first in alphabetical order goes next. std::greater
    // makes the priority queue give the smallest name first, not the largest.
    std::priority_queue<std::string, std::vector<std::string>, std::greater<>> ready;
    for (const auto& [name, count] : waiting) {
        if (count == 0) ready.push(name);
    }
    std::vector<std::string> order;
    while (!ready.empty()) {
        std::string name = ready.top();
        ready.pop();
        order.push_back(name);
        for (const std::string& user : users[name]) {
            if (--waiting[user] == 0) ready.push(user);
        }
    }

    if (order.size() < modules.size()) {
        // Every module left over imports another one left over, so following first
        // imports from the first module left over must come back to a module.
        // waiting is a std::map, so its first module left over is the first by name.
        auto is_left = [&](const std::string& name) { return waiting[name] > 0; };
        auto start = std::find_if(waiting.begin(), waiting.end(),
                                  [](const auto& entry) { return entry.second > 0; });
        std::vector<std::string> path{start->first};
        std::map<std::string, std::size_t> seen{{start->first, 0}};
        while (true) {
            const auto& imports = modules[path.back()].imports;
            std::string next = *std::find_if(imports.begin(), imports.end(), is_left);
            if (auto found = seen.find(next); found != seen.end()) {
                std::cout << "error: import cycle: ";
                for (std::size_t i = found->second; i < path.size(); i++) {
                    std::cout << path[i] << " -> ";
                }
                std::cout << next << '\n';
                return 0;
            }
            seen[next] = path.size();
            path.push_back(next);
        }
    }

    std::cout << "build order: ";
    for (std::size_t i = 0; i < order.size(); i++) std::cout << (i ? ", " : "") << order[i];
    std::cout << (order.empty() ? "nothing to build" : "") << '\n';
    for (const auto& [number, name] : queries) {
        if (!modules.contains(name)) {
            std::cout << "error: line " << number << ": no module called " << name << '\n';
            continue;
        }
        // Everything that imports the changed module, directly or not, comes after
        // it in the build order, so one pass through the order finds them all.
        std::set<std::string> affected{name};
        std::cout << "rebuild " << name << ": " << name;
        for (const std::string& module : order) {
            if (affected.contains(module)) continue;
            for (const std::string& import : modules[module].imports) {
                if (affected.contains(import)) {
                    affected.insert(module);
                    std::cout << ", " << module;
                    break;
                }
            }
        }
        std::cout << '\n';
    }
}
