#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

constexpr long long overdraft_limit = 100;
constexpr long long overdraft_fee = 5;
constexpr int savings_withdrawals = 2;

class AccountError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

// What every account has. Each kind fills in its own rules.
class Account {
  public:
    explicit Account(std::string name) : name_(std::move(name)) {}
    virtual ~Account() = default;

    const std::string &name() const { return name_; }
    long long balance() const { return balance_; }

    void deposit(long long amount) { balance_ += amount; }

    virtual void withdraw(long long amount) {
        check_withdrawal(amount);
        balance_ -= amount;
    }

    virtual std::string month_end() = 0;

  protected:
    virtual void check_withdrawal(long long amount) const = 0;

    long long balance_ = 0;

  private:
    std::string name_;
};

class Checking : public Account {
  public:
    using Account::Account;

    std::string month_end() override {
        if (balance_ < 0) {
            balance_ -= overdraft_fee;
            return "fee " + std::to_string(overdraft_fee);
        }
        return "no fee";
    }

  protected:
    void check_withdrawal(long long amount) const override {
        if (balance_ - amount < -overdraft_limit) {
            throw AccountError("over the overdraft limit of " +
                               std::to_string(overdraft_limit));
        }
    }
};

class Savings : public Account {
  public:
    using Account::Account;

    void withdraw(long long amount) override {
        Account::withdraw(amount);
        withdrawals_++;
    }

    std::string month_end() override {
        long long interest = balance_ / 100;
        balance_ += interest;
        withdrawals_ = 0;
        return "interest " + std::to_string(interest);
    }

  protected:
    void check_withdrawal(long long amount) const override {
        if (withdrawals_ == savings_withdrawals) {
            throw AccountError("only " + std::to_string(savings_withdrawals) +
                               " withdrawals a month");
        }
        if (amount > balance_) {
            throw AccountError("insufficient funds");
        }
    }

  private:
    int withdrawals_ = 0;
};

std::unique_ptr<Account> make_account(const std::string &kind,
                                      const std::string &name) {
    if (kind == "checking") {
        return std::make_unique<Checking>(name);
    }
    if (kind == "savings") {
        return std::make_unique<Savings>(name);
    }
    throw AccountError("unknown account kind: " + kind);
}

class Bank {
  public:
    void run(const std::vector<std::string> &words) {
        if (words.size() == 3 && words[0] == "open") {
            open(words[1], words[2]);
        } else if (words.size() == 3 && words[0] == "deposit") {
            Account &account = find(words[1]);
            long long amount = std::stoll(words[2]);
            account.deposit(amount);
            std::cout << account.name() << ": deposited " << amount
                      << ", balance " << account.balance() << '\n';
        } else if (words.size() == 3 && words[0] == "withdraw") {
            Account &account = find(words[1]);
            long long amount = std::stoll(words[2]);
            try {
                account.withdraw(amount);
            } catch (const AccountError &error) {
                throw AccountError(account.name() + ": " + error.what());
            }
            std::cout << account.name() << ": withdrew " << amount
                      << ", balance " << account.balance() << '\n';
        } else if (words.size() == 1 && words[0] == "month") {
            for (const auto &account : accounts_) {
                std::string change = account->month_end();
                std::cout << account->name() << ": " << change
                          << ", balance " << account->balance() << '\n';
            }
        }
    }

  private:
    void open(const std::string &kind, const std::string &name) {
        std::unique_ptr<Account> account = make_account(kind, name);
        for (const auto &other : accounts_) {
            if (other->name() == name) {
                throw AccountError(name + " already has an account");
            }
        }
        accounts_.push_back(std::move(account));
        std::cout << name << ": opened a " << kind << " account\n";
    }

    Account &find(const std::string &name) {
        for (const auto &account : accounts_) {
            if (account->name() == name) {
                return *account;
            }
        }
        throw AccountError("no account named " + name);
    }

    std::vector<std::unique_ptr<Account>> accounts_;
};

int main() {
    Bank bank;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::vector<std::string> words;
        for (std::string word; in >> word;) {
            words.push_back(word);
        }
        try {
            bank.run(words);
        } catch (const AccountError &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
}
