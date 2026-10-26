#include <iostream>
#include <string_view>

std::string_view describe(int code) {
    switch (code) {
        case 200:
            return "ok";  // return leaves the switch, so no break is needed
        case 301:
        case 302:
        case 307:
        case 308:  // several labels can share one body
            return "redirect";
        case 404:
            return "not found";
        default:
            break;
    }
    // Case labels must be single constants, so ranges need an if.
    if (code >= 400 && code <= 499) {
        return "client error";
    }
    if (code >= 500 && code <= 599) {
        return "server error";
    }
    return "unknown";
}

int main() {
    for (int code : {200, 301, 308, 404, 418, 500, 503, 99}) {
        std::cout << code << ": " << describe(code) << '\n';
    }
}
