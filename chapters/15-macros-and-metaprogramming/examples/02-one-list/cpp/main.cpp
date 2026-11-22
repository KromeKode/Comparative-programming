#include <array>
#include <iostream>
#include <optional>
#include <string_view>

// The same X-macro as in C. Until C++26 adds reflection, an enum can't list
// its own names, so the list is still the usual way to keep them in step.
#define STATUSES(X)                    \
    X(OK, 200, "OK")                   \
    X(MOVED, 301, "Moved Permanently") \
    X(NOT_FOUND, 404, "Not Found")     \
    X(TEAPOT, 418, "I'm a teapot")     \
    X(SERVER_ERROR, 500, "Internal Server Error")

enum class Status {
#define X(name, code, text) name = code,
    STATUSES(X)
#undef X
};

struct StatusInfo {
    Status status;
    std::string_view name;
    std::string_view text;

    int code() const { return static_cast<int>(status); }
};

// One table built from the list. The rest of the program is ordinary C++.
constexpr std::array all_statuses{
#define X(name, code, text) StatusInfo{Status::name, #name, text},
    STATUSES(X)
#undef X
};

constexpr std::optional<StatusInfo> find_code(int code) {
    for (const StatusInfo& info : all_statuses) {
        if (static_cast<int>(info.status) == code) return info;
    }
    return std::nullopt;
}

constexpr std::optional<StatusInfo> find_name(std::string_view name) {
    for (const StatusInfo& info : all_statuses) {
        if (info.name == name) return info;
    }
    return std::nullopt;
}

// The lookups are constexpr, so the compiler can run them and check the
// answer before the program exists (section 15.6).
static_assert(find_code(418)->name == "TEAPOT");

int main() {
    std::cout << all_statuses.size() << " statuses\n";
    for (const StatusInfo& info : all_statuses) {
        std::cout << info.code() << " " << info.name << ": " << info.text << "\n";
    }
    for (int code : {404, 999}) {
        auto info = find_code(code);
        std::cout << "lookup " << code << ": " << (info ? info->text : "unknown") << "\n";
    }
    for (std::string_view name : {"TEAPOT", "BREWING"}) {
        auto info = find_name(name);
        std::cout << "lookup " << name << ": ";
        if (info) {
            std::cout << info->code() << "\n";
        } else {
            std::cout << "unknown\n";
        }
    }
}
