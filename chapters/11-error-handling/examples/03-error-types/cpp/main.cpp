#include <iostream>
#include <stdexcept>
#include <string>

// The base class: catch it to handle every kind of withdraw failure.
struct WithdrawError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct InvalidAmount : WithdrawError {
    explicit InvalidAmount(long amount)
        : WithdrawError("invalid amount: " + std::to_string(amount)) {}
};

struct InsufficientFunds : WithdrawError {
    long needed;
    long available;
    InsufficientFunds(long needed, long available)
        : WithdrawError("insufficient funds: need " + std::to_string(needed) +
                        ", have " + std::to_string(available)),
          needed(needed), available(available) {}
};

struct AccountFrozen : WithdrawError {
    AccountFrozen() : WithdrawError("account is frozen") {}
};

struct Account {
    long balance;
    bool frozen = false;

    void withdraw(long amount) {
        if (amount <= 0) {
            throw InvalidAmount(amount);
        }
        if (frozen) {
            throw AccountFrozen();
        }
        if (amount > balance) {
            throw InsufficientFunds(amount, balance);
        }
        balance -= amount;
    }
};

void attempt(Account &account, long amount) {
    std::cout << "withdraw " << amount << ": ";
    try {
        account.withdraw(amount);
        std::cout << "ok, " << account.balance << " left\n";
    } catch (const InsufficientFunds &error) {
        // The exception carries data, so the caller can recover.
        std::cout << "error: " << error.what() << '\n';
        account.withdraw(error.available);
        std::cout << "  took " << error.available << " instead, "
                  << account.balance << " left\n";
    } catch (const AccountFrozen &error) {
        std::cout << "error: " << error.what() << '\n';
        std::cout << "  call the bank to unfreeze it\n";
    } catch (const WithdrawError &error) {
        // Catch blocks are tried in order, so the base class goes last.
        std::cout << "error: " << error.what() << '\n';
    }
}

int main() {
    Account account{100};
    std::cout << "start with " << account.balance << '\n';
    for (long amount : {30, -5, 100}) {
        attempt(account, amount);
    }
    std::cout << "freeze the account\n";
    account.frozen = true;
    attempt(account, 10);
}
