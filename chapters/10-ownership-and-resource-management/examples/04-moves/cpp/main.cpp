#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// Says which constructor made it, and how many ints it had to copy.
class Loud {
  public:
    explicit Loud(std::vector<int> data) : data_(std::move(data)) {}
    Loud(const Loud &other) : data_(other.data_) {
        std::cout << "  copied " << data_.size() << " ints\n";
    }
    Loud(Loud &&other) noexcept : data_(std::move(other.data_)) {
        std::cout << "  moved, copied 0 ints\n";
    }
    Loud &operator=(const Loud &) = delete;
    Loud &operator=(Loud &&) = delete;
    std::size_t size() const { return data_.size(); }

  private:
    std::vector<int> data_;
};

// Takes its argument by value, so the caller decides: copy or move.
std::size_t keep(Loud loud) { return loud.size(); }

Loud make_loud() {
    Loud made({1, 2, 3});
    // Compilers build made in the caller's place, so this usually neither
    // copies nor moves. Where they can't, return moves.
    return made;
}

int main() {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b = a;            // a copy
    std::vector<int> c = std::move(a); // a move: c takes a's heap block
    std::cout << "copy then move: a has " << a.size() << ", b has "
              << b.size() << ", c has " << c.size() << '\n';

    // A unique_ptr can only be moved. `auto q = p;` does not compile.
    auto p = std::make_unique<int>(7);
    auto q = std::move(p);
    std::cout << "after the move, p is " << (p ? "full" : "empty")
              << " and *q = " << *q << '\n';

    std::cout << "passing by value:\n";
    Loud big({1, 2, 3, 4, 5});
    keep(big);
    keep(std::move(big));
    // big is still here, but moved-from: valid, with an unspecified value.

    std::cout << "returning a local:\n";
    Loud made = make_loud();
    std::cout << "  made has " << made.size() << " ints\n";

    // std::move only asks for a move. A const object cannot be moved from,
    // so this quietly copies.
    const std::vector<int> fixed{1, 2};
    std::vector<int> d = std::move(fixed);
    std::cout << "std::move on a const vector copied it: fixed has "
              << fixed.size() << ", d has " << d.size() << '\n';
}
