#include <format>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

struct Cash {};
struct Card {
    std::string last4;
};
struct Voucher {
    std::string code;
    long cents;
};

// A variant holds exactly one of its types, and remembers which one.
using Payment = std::variant<Cash, Card, Voucher>;

// Combines several lambdas into one object that std::visit can call.
template <class... Fs> struct overloaded : Fs... {
    using Fs::operator()...;
};

std::string money(long cents) {
    return std::format("{}.{:02}", cents / 100, cents % 100);
}

std::string describe(const Payment &payment, long total) {
    // std::visit calls the lambda for the type the variant holds.
    return std::visit(
        overloaded{
            [&](const Cash &) { return std::format("cash: pay {} in cash", money(total)); },
            [&](const Card &card) {
                return std::format("card ending {}: charge {} to the card", card.last4,
                                   money(total));
            },
            [&](const Voucher &voucher) {
                if (voucher.cents >= total) {
                    return std::format("voucher {} ({}): covers the order", voucher.code,
                                       money(voucher.cents));
                }
                return std::format("voucher {} ({}): {} left to pay", voucher.code,
                                   money(voucher.cents), money(total - voucher.cents));
            },
        },
        payment);
}

int main() {
    long total = 1250;
    std::vector<Payment> payments = {Cash{}, Card{"4242"}, Voucher{"SPRING", 500},
                                     Voucher{"BIG", 2000}};
    std::cout << "order total: " << money(total) << '\n';
    for (const auto &payment : payments) {
        std::cout << describe(payment, total) << '\n';
    }
}
