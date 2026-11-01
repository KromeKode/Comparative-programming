#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Counts folders as they are destroyed, so the program can report how many
// were freed.
int freed = 0;

// A folder owns its children through unique_ptr. Its parent pointer is a
// plain pointer that owns nothing: the parent owns the child, never the
// other way round, so there is no cycle.
struct Folder {
    std::string name;
    Folder *parent = nullptr;
    std::vector<std::unique_ptr<Folder>> children;

    explicit Folder(std::string n) : name(std::move(n)) {}
    ~Folder() { freed++; }
};

struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

std::string folders(int count) {
    return std::to_string(count) + (count == 1 ? " folder" : " folders");
}

// Finds a folder by name, searching the whole tree from `from` down.
Folder *find(Folder *from, const std::string &name) {
    if (from->name == name) {
        return from;
    }
    for (const auto &child : from->children) {
        if (Folder *found = find(child.get(), name)) {
            return found;
        }
    }
    return nullptr;
}

Folder *lookup(Folder *root, const std::string &name) {
    Folder *found = find(root, name);
    if (found == nullptr) {
        throw Error("no folder \"" + name + '"');
    }
    return found;
}

// Builds the path by following parent pointers up to the root.
std::string path(const Folder *folder) {
    std::vector<std::string> names;
    for (; folder->parent != nullptr; folder = folder->parent) {
        names.push_back(folder->name);
    }
    std::string result;
    for (auto it = names.rbegin(); it != names.rend(); ++it) {
        result += '/' + *it;
    }
    return result.empty() ? "/" : result;
}

// True when folder is inside ancestor, or is ancestor.
bool is_inside(const Folder *folder, const Folder *ancestor) {
    for (; folder != nullptr; folder = folder->parent) {
        if (folder == ancestor) {
            return true;
        }
    }
    return false;
}

// Takes folder out of its parent and returns the unique_ptr that owned it.
std::unique_ptr<Folder> detach(Folder *folder) {
    auto &siblings = folder->parent->children;
    auto it = std::find_if(siblings.begin(), siblings.end(),
                           [&](const auto &c) { return c.get() == folder; });
    std::unique_ptr<Folder> owned = std::move(*it);
    siblings.erase(it);
    return owned;
}

void attach(std::unique_ptr<Folder> folder, Folder *parent) {
    folder->parent = parent;
    parent->children.push_back(std::move(folder));
}

void takes(const std::string &command, std::size_t want, std::size_t got) {
    if (got != want) {
        std::string noun = want == 1 ? " name" : " names";
        throw Error(command + " takes " + std::to_string(want) + noun +
                    ", got " + std::to_string(got));
    }
}

void run(Folder *root, const std::string &command,
         const std::vector<std::string> &args) {
    if (command == "mkdir") {
        takes(command, 2, args.size());
        if (find(root, args[0]) != nullptr) {
            throw Error('"' + args[0] + "\" already exists");
        }
        Folder *parent = lookup(root, args[1]);
        auto folder = std::make_unique<Folder>(args[0]);
        Folder *made = folder.get();
        attach(std::move(folder), parent);
        std::cout << "made " << path(made) << '\n';
    } else if (command == "path") {
        takes(command, 1, args.size());
        std::cout << path(lookup(root, args[0])) << '\n';
    } else if (command == "ls") {
        takes(command, 1, args.size());
        Folder *folder = lookup(root, args[0]);
        std::cout << folder->name << ':';
        for (const auto &child : folder->children) {
            std::cout << ' ' << child->name;
        }
        std::cout << (folder->children.empty() ? " (empty)\n" : "\n");
    } else if (command == "rm") {
        takes(command, 1, args.size());
        Folder *folder = lookup(root, args[0]);
        if (folder == root) {
            throw Error("cannot remove /");
        }
        int before = freed;
        detach(folder); // the returned unique_ptr is destroyed right here
        std::cout << "removed " << args[0] << ", freed "
                  << folders(freed - before) << '\n';
    } else if (command == "mv") {
        takes(command, 2, args.size());
        Folder *folder = lookup(root, args[0]);
        if (folder == root) {
            throw Error("cannot move /");
        }
        Folder *parent = lookup(root, args[1]);
        if (is_inside(parent, folder)) {
            throw Error("cannot move " + args[0] + " into itself");
        }
        attach(detach(folder), parent);
        std::cout << "moved " << args[0] << " to " << path(folder) << '\n';
    } else {
        throw Error("unknown command \"" + command + '"');
    }
}

int main() {
    auto root = std::make_unique<Folder>("/");
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        std::vector<std::string> parts;
        for (std::string part; words >> part;) {
            parts.push_back(part);
        }
        if (parts.empty()) {
            continue;
        }
        try {
            run(root.get(), parts[0], {parts.begin() + 1, parts.end()});
        } catch (const Error &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
    int before = freed;
    root.reset();
    std::cout << "freed " << folders(freed - before) << " at exit\n";
}
