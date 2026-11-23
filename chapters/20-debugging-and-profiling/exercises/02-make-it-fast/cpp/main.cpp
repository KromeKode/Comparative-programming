// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr long long START = 1000;  // what each account starts with

// The Park-Miller generator, which every language's version uses, so that they all
// make up the same transfers from the same seed.
class Random {
public:
    explicit Random(long long seed) : state_(seed) {}
    long long next() { return state_ = state_ * 48271 % 2147483647; }

private:
    long long state_;
};

struct Account {
    std::string name;
    long long balance;
};

std::string account_name(long long i) {
    char name[16];
    std::snprintf(name, sizeof name, "acct%06lld", i);
    return name;
}

// Gives the account with this name.
Account& find(std::vector<Account>& accounts, const std::string& name) {
    for (Account& account : accounts) {
        if (account.name == name) return account;
    }
    throw std::out_of_range(name);
}

long long total(const std::vector<Account>& accounts) {
    long long sum = 0;
    for (const Account& account : accounts) sum += account.balance;
    return sum;
}

int main() {
    long long count, transfers, seed;
    if (!(std::cin >> count >> transfers >> seed)) return 1;
    std::vector<Account> accounts;
    for (long long i = 0; i < count; i++) accounts.push_back({account_name(i), START});

    Random random(seed);
    long long refused = 0;
    for (long long t = 0; t < transfers; t++) {
        Account& source = find(accounts, account_name(random.next() % count));
        Account& target = find(accounts, account_name(random.next() % count));
        long long amount = random.next() % 500 + 1;
        if (source.balance < amount) {
            refused++;
        } else {
            source.balance -= amount;
            target.balance += amount;
        }
        // Money is never made or lost, so the total stays the same.
        if (total(accounts) != count * START) {
            std::cerr << "error: the total has changed\n";
            return 1;
        }
    }

    std::cout << "transfers: " << transfers << ", refused: " << refused << '\n';
    std::cout << "total: " << total(accounts) << '\n';
    std::sort(accounts.begin(), accounts.end(), [](const Account& a, const Account& b) {
        return a.balance != b.balance ? a.balance > b.balance : a.name < b.name;
    });
    for (std::size_t i = 0; i < accounts.size() && i < 5; i++) {
        std::cout << accounts[i].name << ' ' << accounts[i].balance << '\n';
    }
}
