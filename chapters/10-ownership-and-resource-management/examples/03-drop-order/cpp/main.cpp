#include <iostream>
#include <optional>
#include <string>
#include <utility>

// Prints when it is made and when it is destroyed.
class Noisy {
  public:
    explicit Noisy(std::string name) : name_(std::move(name)) {
        std::cout << "  make " << name_ << '\n';
    }
    ~Noisy() { std::cout << "  drop " << name_ << '\n'; }
    Noisy(const Noisy &) = delete;
    Noisy &operator=(const Noisy &) = delete;

  private:
    std::string name_;
};

// The destructor's body runs first. Then the members are destroyed, in the
// reverse of the order they are declared in.
struct Pair {
    Noisy first{"pair.first"};
    Noisy second{"pair.second"};
    ~Pair() { std::cout << "  drop pair\n"; }
};

int main() {
    std::cout << "locals go in the reverse order:\n";
    {
        Noisy a("a");
        Noisy b("b");
        Noisy c("c");
    }

    std::cout << "an inner scope ends first:\n";
    {
        Noisy outer("outer");
        {
            Noisy inner("inner");
        }
        std::cout << "  inner is gone, outer is not\n";
    }

    std::cout << "a struct, then its members:\n";
    {
        Pair pair;
    }

    std::cout << "a temporary lasts until the end of its statement:\n";
    Noisy("temporary");
    std::cout << "  the temporary is gone\n";

    std::cout << "ending a lifetime early:\n";
    {
        std::optional<Noisy> early;
        early.emplace("early");
        early.reset();
        std::cout << "  early is gone before its scope ends\n";
    }
}
