#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr int max_open = 2;
constexpr std::size_t max_lines = 8;

struct DiskError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct File {
    std::vector<std::string> lines;
    bool locked = false;
};

struct Disk {
    std::map<std::string, File> files;
    int open_count = 0;

    std::size_t lines_used() const {
        std::size_t total = 0;
        for (const auto &[name, file] : files) {
            total += file.lines.size();
        }
        return total;
    }
};

// Opening happens in the constructor and closing in the destructor, so a
// Handle that exists is open, and it closes however its scope ends.
class Handle {
  public:
    Handle(Disk &disk, const std::string &name, bool writing)
        : disk_(disk), name_(name) {
        if (disk.open_count == max_open) {
            throw DiskError("too many open files");
        }
        auto found = disk.files.find(name);
        if (writing) {
            if (found != disk.files.end() && found->second.locked) {
                throw DiskError('"' + name + "\" is read-only");
            }
            disk.files[name].lines.clear();
        } else if (found == disk.files.end()) {
            throw DiskError("no file \"" + name + '"');
        }
        disk.open_count++;
        std::cout << "open " << name << '\n';
    }

    ~Handle() {
        disk_.open_count--;
        std::cout << "close " << name_ << '\n';
    }

    // A copy would close the file a second time.
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    std::vector<std::string> lines() const {
        return disk_.files.at(name_).lines;
    }

    void write(const std::string &line) {
        if (disk_.lines_used() == max_lines) {
            throw DiskError("disk full");
        }
        disk_.files.at(name_).lines.push_back(line);
    }

  private:
    Disk &disk_;
    std::string name_;
};

std::string lines(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " line" : " lines");
}

void write_file(Disk &disk, const std::string &name,
                const std::vector<std::string> &words) {
    {
        Handle target(disk, name, true);
        for (const std::string &word : words) {
            target.write(word);
        }
    }
    std::cout << "wrote " << lines(words.size()) << '\n';
}

void cat_file(Disk &disk, const std::string &name) {
    Handle source(disk, name, false);
    for (const std::string &line : source.lines()) {
        std::cout << "  " << line << '\n';
    }
}

void copy_file(Disk &disk, const std::string &from, const std::string &to) {
    if (from == to) {
        throw DiskError("cannot copy \"" + from + "\" onto itself");
    }
    std::size_t copied = 0;
    {
        Handle source(disk, from, false);
        Handle target(disk, to, true);
        for (const std::string &line : source.lines()) {
            target.write(line);
            copied++;
        }
    }
    std::cout << "copied " << lines(copied) << '\n';
}

void lock_file(Disk &disk, const std::string &name) {
    auto found = disk.files.find(name);
    if (found == disk.files.end()) {
        throw DiskError("no file \"" + name + '"');
    }
    found->second.locked = true;
    std::cout << "locked " << name << '\n';
}

void takes(const std::string &command, std::size_t want, std::size_t got) {
    if (got != want) {
        std::string noun = want == 1 ? " name" : " names";
        throw DiskError(command + " takes " + std::to_string(want) + noun +
                        ", got " + std::to_string(got));
    }
}

void run(Disk &disk, const std::vector<std::string> &parts) {
    const std::string &command = parts[0];
    std::vector<std::string> args(parts.begin() + 1, parts.end());
    if (command == "write") {
        if (args.empty()) {
            throw DiskError("write needs a file name");
        }
        write_file(disk, args[0], {args.begin() + 1, args.end()});
    } else if (command == "cat") {
        takes(command, 1, args.size());
        cat_file(disk, args[0]);
    } else if (command == "copy") {
        takes(command, 2, args.size());
        copy_file(disk, args[0], args[1]);
    } else if (command == "lock") {
        takes(command, 1, args.size());
        lock_file(disk, args[0]);
    } else {
        throw DiskError("unknown command \"" + command + '"');
    }
}

int main() {
    Disk disk;
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
            run(disk, parts);
        } catch (const DiskError &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
    std::cout << "files still open: " << disk.open_count << '\n';
}
