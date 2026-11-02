#include <concepts>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Animal {
public:
    virtual ~Animal() = default;
    virtual std::string name() const = 0;
    virtual std::string sound() const = 0;
};

// final: nothing can derive from Cat, so a call through a Cat& can only mean
// Cat's own method, and the compiler can call it directly.
class Cat final : public Animal {
public:
    explicit Cat(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    std::string sound() const override { return "meow"; }
    std::string purr() const { return "prrr"; }

private:
    std::string name_;
};

class Dog final : public Animal {
public:
    explicit Dog(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    std::string sound() const override { return "woof"; }

private:
    std::string name_;
};

// Static dispatch. The compiler makes one copy of chorus for Cat and another
// for Dog. All the animals in one call must be the same type.
template <std::derived_from<Animal> A>
std::string chorus(const std::vector<A>& animals) {
    std::string text;
    for (const A& animal : animals) {
        if (!text.empty()) text += ", ";
        text += animal.name() + " says " + animal.sound();
    }
    return text;
}

// Dynamic dispatch. There is one copy, and each call goes through the object's
// table of virtual functions. The animals can be any mix.
std::string chorus_all(const std::vector<const Animal*>& animals) {
    std::string text;
    for (const Animal* animal : animals) {
        if (!text.empty()) text += ", ";
        text += animal->name() + " says " + animal->sound();
    }
    return text;
}

// A template hands back the caller's own type, so the caller can still use
// what only a Cat has.
template <std::derived_from<Animal> A>
const A& longest_name(const std::vector<A>& animals) {
    const A* best = &animals.front();
    for (const A& animal : animals) {
        if (animal.name().size() > best->name().size()) best = &animal;
    }
    return *best;
}

int main() {
    std::vector<Cat> cats{Cat("Tom"), Cat("Kitty")};
    std::vector<Dog> dogs{Dog("Rex")};
    std::cout << "cats: " << chorus(cats) << "\n";
    std::cout << "dogs: " << chorus(dogs) << "\n";

    std::vector<const Animal*> everyone{&cats[0], &dogs[0], &cats[1]};
    std::cout << "everyone: " << chorus_all(everyone) << "\n";

    const Cat& cat = longest_name(cats);
    std::cout << "longest name: " << cat.name() << ", who purrs " << cat.purr() << "\n";
}
