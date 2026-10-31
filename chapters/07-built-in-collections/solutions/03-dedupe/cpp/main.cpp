#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        std::unordered_set<std::string> seen;
        std::string output;
        for (std::string word; words >> word;) {
            // insert returns a pair; .second is true if the word was not there yet.
            if (seen.insert(word).second) {
                output += (output.empty() ? "" : " ") + word;
            }
        }
        if (!output.empty()) {
            std::cout << output << '\n';
        }
    }
}
