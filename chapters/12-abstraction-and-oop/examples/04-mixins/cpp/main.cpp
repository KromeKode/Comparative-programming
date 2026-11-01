#include <iostream>
#include <string>
#include <utility>
#include <vector>

// A mixin in C++ is often a class template that takes the class using it as a
// parameter (the curiously recurring template pattern, or CRTP). The mixin
// casts itself to that class to call its name(), with no virtual functions.
template <typename Self> class Describe {
  public:
    std::string describe() const { return "this is the " + self().name(); }

  private:
    const Self &self() const { return static_cast<const Self &>(*this); }
};

template <typename Self> class Logged {
  public:
    void log(const std::string &event) {
        history_.push_back(self().name() + ": " + event);
    }

    const std::vector<std::string> &history() const { return history_; }

  private:
    const Self &self() const { return static_cast<const Self &>(*this); }

    std::vector<std::string> history_;
};

class Door : public Describe<Door>, public Logged<Door> {
  public:
    explicit Door(std::string name) : name_(std::move(name)) {}

    const std::string &name() const { return name_; }
    void open() { log("opened"); }
    void close() { log("closed"); }

  private:
    std::string name_;
};

class Light : public Describe<Light>, public Logged<Light> {
  public:
    explicit Light(std::string name) : name_(std::move(name)) {}

    const std::string &name() const { return name_; }

    void switch_on() {
        on_ = true;
        log("switched on");
    }

    // Light's describe hides the mixin's, and reuses it.
    std::string describe() const {
        return Describe<Light>::describe() + ", and it is " +
               (on_ ? "on" : "off");
    }

  private:
    std::string name_;
    bool on_ = false;
};

// Door and Light share no base class, so a template prints either one.
template <typename Thing> void print(const Thing &thing) {
    std::cout << thing.describe() << '\n';
    for (const std::string &line : thing.history()) {
        std::cout << "  " << line << '\n';
    }
}

int main() {
    Door door("front door");
    Light light("porch light");
    door.open();
    door.close();
    light.switch_on();
    print(door);
    print(light);
}
