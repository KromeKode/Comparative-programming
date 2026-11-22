#include <algorithm>
#include <cctype>
#include <future>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using Counts = std::unordered_map<std::string, long>;

std::string plural(long count, const std::string& noun) {
    return std::to_string(count) + " " + noun + (count == 1 ? "" : "s");
}

bool is_letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

// Runs on a thread of its own. It reads only its own lines, which nothing
// changes while it runs, and returns a map of its own.
Counts count_words(std::span<const std::string> lines) {
    Counts counts;
    for (const auto& line : lines) {
        for (size_t at = 0; at < line.size();) {
            if (!is_letter(line[at])) {
                at++;
                continue;
            }
            std::string word;
            while (at < line.size() && is_letter(line[at])) {
                word += static_cast<char>(std::tolower(static_cast<unsigned char>(line[at++])));
            }
            counts[word]++;
        }
    }
    return counts;
}

long total_of(const Counts& counts) {
    long total = 0;
    for (const auto& [word, count] : counts) total += count;
    return total;
}

bool is_number(const std::string& word) {
    return !word.empty() && std::ranges::all_of(word, [](char c) { return c >= '0' && c <= '9'; });
}

int main() {
    std::vector<std::string> lines;
    for (std::string line; std::getline(std::cin, line);) lines.push_back(line);
    std::istringstream first(lines.empty() ? "" : lines[0]);
    std::vector<std::string> words;
    for (std::string word; first >> word;) words.push_back(word);
    if (words.size() != 2 || !is_number(words[0]) || !is_number(words[1])) {
        std::cout << "error: the first line must be two numbers, workers and top\n";
        return 0;
    }
    long workers = std::stol(words[0]), top = std::stol(words[1]);
    if (workers < 1 || workers > 16) {
        std::cout << "error: workers must be from 1 to 16\n";
        return 0;
    }

    // Each worker gets text.size() / workers lines, and the first few get one
    // more, so that every line is counted once.
    std::span<const std::string> text(lines);
    if (!text.empty()) text = text.subspan(1);
    std::vector<std::span<const std::string>> chunks;
    size_t start = 0;
    for (long i = 0; i < workers; i++) {
        size_t size = text.size() / workers + (static_cast<size_t>(i) < text.size() % workers);
        chunks.push_back(text.subspan(start, size));
        start += size;
    }

    // std::async with std::launch::async runs each call on a new thread, and
    // get waits for its result. A span is a pointer and a length, so the
    // threads read the lines in place, without copying them.
    std::vector<std::future<Counts>> futures;
    for (auto chunk : chunks) futures.push_back(std::async(std::launch::async, count_words, chunk));
    std::vector<Counts> counts;
    for (auto& future : futures) counts.push_back(future.get());

    std::cout << plural(workers, "worker") << ", " << plural(text.size(), "line") << "\n";
    Counts total;
    for (size_t i = 0; i < chunks.size(); i++) {
        std::cout << "worker " << i << ": " << plural(chunks[i].size(), "line") << ", "
                  << plural(total_of(counts[i]), "word") << "\n";
        for (const auto& [word, count] : counts[i]) total[word] += count;
    }
    std::cout << plural(total_of(total), "word") << ", " << total.size() << " different\n";

    std::vector<std::pair<std::string, long>> sorted(total.begin(), total.end());
    std::ranges::sort(sorted, [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (size_t i = 0; i < sorted.size() && static_cast<long>(i) < top; i++) {
        std::cout << sorted[i].first << ": " << sorted[i].second << "\n";
    }
}
