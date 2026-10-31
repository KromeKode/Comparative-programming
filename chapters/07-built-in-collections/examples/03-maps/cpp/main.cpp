#include <iostream>
#include <map>
#include <string>

// std::map keeps its keys sorted. std::unordered_map is a hash table: usually faster,
// but its order is unspecified.
void print(const std::string& label, const std::map<std::string, int>& stock) {
    std::cout << label << ':';
    for (const auto& [key, value] : stock) {
        std::cout << ' ' << key << '=' << value;
    }
    std::cout << '\n';
}

int main() {
    std::map<std::string, int> stock = {{"apple", 3}, {"banana", 5}};
    print("stock", stock);

    stock["cherry"] = 7;
    print("add cherry", stock);

    std::cout << "banana: " << stock.at("banana") << '\n';  // at() throws if missing
    // stock["durian"] would add durian with the value 0. find() only looks.
    auto durian = stock.find("durian");
    if (durian == stock.end()) {
        std::cout << "durian: 0 (missing)\n";
    }

    stock["apple"] += 2;
    print("apple += 2", stock);
    stock.erase("banana");
    print("remove banana", stock);

    std::cout << "has cherry: " << (stock.contains("cherry") ? "yes" : "no") << '\n';
    std::cout << "size: " << stock.size() << '\n';
}
