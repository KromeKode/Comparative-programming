#include <iostream>
#include <string>
#include <vector>

// A bag of items. add_all is written in terms of add, and add is virtual, so a
// subclass that overrides add changes what add_all does.
class Bag {
  public:
    virtual ~Bag() = default;

    virtual void add(const std::string &item) { items_.push_back(item); }

    virtual void add_all(const std::vector<std::string> &items) {
        for (const std::string &item : items) {
            add(item); // calls the subclass's add, if it has one
        }
    }

    std::size_t size() const { return items_.size(); }

  private:
    std::vector<std::string> items_;
};

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag : public Bag {
  public:
    void add(const std::string &item) override {
        count++;
        Bag::add(item);
    }

    void add_all(const std::vector<std::string> &items) override {
        count += items.size();
        Bag::add_all(items); // which calls add, which counts again
    }

    std::size_t count = 0;
};

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  public:
    void add(const std::string &item) {
        count++;
        bag_.add(item);
    }

    void add_all(const std::vector<std::string> &items) {
        count += items.size();
        bag_.add_all(items); // the inner Bag calls its own add
    }

    std::size_t size() const { return bag_.size(); }

    std::size_t count = 0;

  private:
    Bag bag_;
};

template <typename CountingBag> void fill(const std::string &label) {
    CountingBag bag;
    bag.add_all({"apple", "pear", "plum"});
    bag.add("fig");
    std::cout << label << ": the bag holds " << bag.size()
              << " items, the count says " << bag.count << '\n';
}

int main() {
    fill<InheritedCountingBag>("inheritance");
    fill<ComposedCountingBag>("composition");
}
