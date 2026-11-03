#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>

// A ring keeps the last three items pushed into it, oldest first.
class Ring {
public:
    // A C++ iterator is a position. A range-for needs only *, ++, and ==, but
    // these two type names and post-increment also make it a std::input_iterator.
    class Iterator {
    public:
        using value_type = int;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        Iterator(const Ring* ring, std::size_t index) : ring_(ring), index_(index) {}

        int operator*() const {
            return ring_->items_[(ring_->start_ + index_) % ring_->items_.size()];
        }

        Iterator& operator++() {
            index_++;
            return *this;
        }

        void operator++(int) { ++*this; }

        bool operator==(const Iterator& other) const { return index_ == other.index_; }

    private:
        const Ring* ring_ = nullptr;
        std::size_t index_ = 0;
    };

    void push(int item) {
        items_[(start_ + len_) % items_.size()] = item;
        if (len_ < items_.size()) {
            len_++;
        } else {
            start_ = (start_ + 1) % items_.size();  // the oldest item is overwritten
        }
    }

    // begin and end are what a range-for calls.
    Iterator begin() const { return {this, 0}; }
    Iterator end() const { return {this, len_}; }

private:
    std::array<int, 3> items_{};
    std::size_t start_ = 0, len_ = 0;
};

static_assert(std::ranges::input_range<Ring>);  // so std::ranges algorithms accept it

int main() {
    Ring ring;
    for (int i = 1; i <= 5; i++) ring.push(i);

    std::cout << "loop:";
    for (int item : ring) std::cout << " " << item;
    std::cout << "\nagain:";
    for (int item : ring) std::cout << " " << item;  // begin() starts a new position
    std::cout << "\n";

    std::cout << "by hand:";
    const char* sep = " ";
    for (auto it = ring.begin(); it != ring.end(); ++it, sep = ", then ") {
        std::cout << sep << *it;
    }
    std::cout << ", then nothing\n";

    // Moving the position forward by hand, then looping from there.
    auto it = ring.begin();
    std::cout << "resumed: took " << *it << ", then the loop got";
    ++it;
    for (int item : std::ranges::subrange(it, ring.end())) std::cout << " " << item;
    std::cout << "\n";
}
