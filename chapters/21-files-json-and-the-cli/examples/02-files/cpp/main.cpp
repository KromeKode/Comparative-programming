// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

// A streams error doesn't say what went wrong, but on Linux errno does.
[[noreturn]] void throw_errno() {
    throw std::system_error(errno, std::generic_category());
}

// Rewrites the file without its repeated lines, and returns how many there were.
std::size_t dedup(const std::string& path) {
    std::vector<std::string> lines;
    {
        std::ifstream in(path, std::ios::binary);  // binary: no "\r\n" changes on Windows
        if (!in) throw_errno();
        // getline splits at each '\n' and drops it; a last line without one still counts.
        for (std::string line; std::getline(in, line);) lines.push_back(line);
        // Opening a folder works, and reading it fails, which sets badbit.
        if (in.bad()) throw_errno();
    }  // in closes here, at the end of its scope
    std::unordered_set<std::string> seen;
    std::vector<std::string> kept;
    for (const std::string& line : lines) {
        if (seen.insert(line).second) kept.push_back(line);  // second: it was new
    }
    std::size_t removed = lines.size() - kept.size();
    if (removed == 0) return 0;

    // The standard library can't make a file with a fresh name, so it's FILE.tmp.
    std::string temporary = path + ".tmp";
    try {
        {
            std::ofstream out(temporary, std::ios::binary);
            if (!out) throw_errno();
            for (const std::string& line : kept) out << line << '\n';
            // C++ has no fsync: an ofstream doesn't give the file descriptor it needs.
            out.close();
            if (!out) throw_errno();
        }
        fs::permissions(temporary, fs::status(path).permissions());
        fs::rename(temporary, path);
    } catch (...) {
        std::error_code ignored;  // remove reports into it, instead of throwing
        fs::remove(temporary, ignored);
        throw;
    }
    return removed;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: dedup FILE...\n";
        return 2;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        std::string path = argv[i];
        try {
            std::size_t removed = dedup(path);
            std::cout << path << ": removed " << removed << (removed == 1 ? " line\n" : " lines\n");
        } catch (const std::system_error& error) {  // fs::filesystem_error is one too
            // code().message() is the system's message, such as "No such file or directory".
            std::cerr << "dedup: " << path << ": " << error.code().message() << '\n';
            status = 1;
        }
    }
    return status;
}
