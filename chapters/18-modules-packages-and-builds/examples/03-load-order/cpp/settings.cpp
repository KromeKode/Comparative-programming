#include "settings.hpp"

#include <iostream>

namespace settings {

namespace {

std::string load() {
    std::cout << "settings: loading\n";
    return "Hello";
}

}  // namespace

// A global whose value needs code is set up before main runs. With several
// files, the order in which their globals are set up isn't defined.
const std::string greeting = load();

}  // namespace settings
