// Prints a report of each student's scores. ../README.md says what it should print.
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Gives total / count, rounded to the nearest whole number, with halves up.
long rounded_average(long total, long count) { return (2 * total + count) / (2 * count); }

int main(int argc, char** argv) {
    // The input is the file named on the command line, if there is one, so that a
    // debugger can keep the terminal to itself.
    std::ifstream file;
    if (argc > 1) {
        file.open(argv[1]);
        if (!file) {
            std::cerr << "can't open " << argv[1] << '\n';
            return 1;
        }
    }
    std::istream& in = argc > 1 ? file : std::cin;
    std::vector<std::string> names;  // in the order they first appear
    std::map<std::string, std::vector<int>> scores;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        std::string name;
        if (!(fields >> name)) continue;
        if (!scores.contains(name)) names.push_back(name);
        std::vector<int> marks;
        for (int mark; fields >> mark;) marks.push_back(mark);
        std::vector<int>& all = scores[name];
        all.insert(all.end(), marks.begin(), marks.end());
    }

    std::string best_name = "-";
    long best_average = -1;
    long class_total = 0, class_count = 0;
    for (const std::string& name : names) {
        const std::vector<int>& marks = scores[name];
        long total = 0;
        for (int mark : marks) total += mark;
        class_total += total;
        class_count += static_cast<long>(marks.size());
        if (marks.empty()) {
            std::cout << name << " -\n";
            continue;
        }
        long average = rounded_average(total, static_cast<long>(marks.size()));
        std::cout << name << ' ' << average << '\n';
        if (average > best_average) {
            best_name = name;
            best_average = average;
        }
    }

    if (class_count == 0) {
        std::cout << "class average: -\n";
    } else {
        std::cout << "class average: " << rounded_average(class_total, class_count) << '\n';
    }
    std::cout << "top: " << best_name << '\n';
}
