#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct Node {
    std::string name;
    std::shared_ptr<Node> next; // owns what it points to
    std::weak_ptr<Node> back;   // does not

    explicit Node(std::string n) : name(std::move(n)) {
        std::cout << "  make " << name << '\n';
    }
    ~Node() { std::cout << "  drop " << name << '\n'; }
};

const char *yes_no(bool flag) { return flag ? "yes" : "no"; }

int main() {
    std::cout << "one owner:\n";
    {
        auto report = std::make_unique<Node>("report");
    }

    std::cout << "two owners:\n";
    {
        auto doc = std::make_shared<Node>("doc");
        auto also = doc;
        std::cout << "  owners: " << doc.use_count() << '\n';
        doc.reset();
        std::cout << "  one owner let go, owners: " << also.use_count() << '\n';
    }

    std::cout << "a weak pointer does not own:\n";
    std::weak_ptr<Node> weak;
    {
        auto cache = std::make_shared<Node>("cache");
        weak = cache;
        std::cout << "  weak can reach cache: " << yes_no(!weak.expired()) << '\n';
    }
    std::cout << "  weak can reach cache: " << yes_no(!weak.expired()) << '\n';

    std::cout << "a cycle of owning links:\n";
    std::weak_ptr<Node> watch;
    {
        auto a = std::make_shared<Node>("a");
        auto b = std::make_shared<Node>("b");
        a->next = b;
        b->next = a;
        watch = a;
    }
    std::cout << "  a and b went out of scope, and a still has "
              << watch.use_count() << " owner\n";
    std::cout << "  break the cycle by hand:\n";
    watch.lock()->next.reset();

    std::cout << "an owning link one way, a weak link back:\n";
    {
        auto a = std::make_shared<Node>("a");
        auto b = std::make_shared<Node>("b");
        a->next = b;
        b->back = a;
        std::cout << "  b's back link reaches " << b->back.lock()->name << '\n';
    }
}
