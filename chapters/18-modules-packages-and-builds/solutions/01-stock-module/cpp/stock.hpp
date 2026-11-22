// The stock module: how many of each item there are.
#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace stock {

class Stock {
public:
    void add(const std::string& item, long long amount);
    // Takes amount of item away, and returns true, if there are that many.
    bool take(const std::string& item, long long amount);
    // How many of item there are: 0 for an item never added.
    long long count(const std::string& item) const;
    // The items there are any of, with their counts, in alphabetical order.
    std::vector<std::pair<std::string, long long>> items() const;

private:
    // Other files see this line, since it is in the header, but only Stock's own
    // functions can use it: main.cpp can't write stock.counts_.
    std::map<std::string, long long> counts_;
};

}  // namespace stock
