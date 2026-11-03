#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Animal {
public:
    virtual ~Animal() = default;
    virtual std::string name() const = 0;
};

class Cat final : public Animal {
public:
    explicit Cat(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }

private:
    std::string name_;
};

class Dog final : public Animal {
public:
    explicit Dog(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }

private:
    std::string name_;
};

std::string names(const std::vector<const Animal*>& animals) {
    std::string text;
    for (const Animal* animal : animals) text += (text.empty() ? "" : ", ") + animal->name();
    return text;
}

// Each Cat* converts to an Animal*, but a vector of one is never a vector of
// the other. Build a new vector, converting each pointer.
std::vector<const Animal*> as_animals(const std::vector<Cat>& cats) {
    std::vector<const Animal*> animals;
    for (const Cat& cat : cats) animals.push_back(&cat);
    return animals;
}

int main() {
    std::vector<Cat> cats{Cat("Tom"), Cat("Kitty")};
    std::vector<const Animal*> animals = as_animals(cats);
    std::cout << "names: " << names(animals) << "\n";

    Dog rex("Rex");
    std::vector<const Animal*> shelter = animals;
    shelter.push_back(&rex);
    std::cout << "shelter: " << names(shelter) << "\n";

    // A function for any two animals works on two cats, because each Cat&
    // converts to an Animal& as it is passed.
    auto by_name = [](const Animal& a, const Animal& b) { return a.name() < b.name(); };
    std::vector<Cat> sorted = cats;
    std::ranges::sort(sorted, by_name);
    std::cout << "sorted by name: " << names(as_animals(sorted)) << "\n";
}
