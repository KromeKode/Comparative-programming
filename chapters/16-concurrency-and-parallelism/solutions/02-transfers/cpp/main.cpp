#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct Transfer {
    size_t source, target;
    long amount;
};

std::string plural(size_t count, const std::string& noun) {
    return std::to_string(count) + " " + noun + (count == 1 ? "" : "s");
}

// The whole numbers on a line, or nothing if a word isn't one.
std::optional<std::vector<long>> numbers(const std::string& line) {
    std::istringstream words(line);
    std::vector<long> result;
    for (std::string word; words >> word;) {
        if (word.find_first_not_of("0123456789") != std::string::npos) return std::nullopt;
        result.push_back(std::stol(word));
    }
    return result;
}

class Bank {
public:
    explicit Bank(std::vector<long> balances)
        : balances_(std::move(balances)), locks_(balances_.size()) {}

    void transfer(const Transfer& t) {
        if (t.source == t.target) return;  // locking one mutex twice is undefined
        // scoped_lock takes both locks without deadlock, whatever order other
        // threads take them in, and releases them at the end of the block.
        std::scoped_lock both(locks_[t.source], locks_[t.target]);
        long source_balance = balances_[t.source];
        long target_balance = balances_[t.target];
        // Lets another thread run between the read and the write, as it could
        // while a real bank waits on its database.
        std::this_thread::yield();
        balances_[t.source] = source_balance - t.amount;
        balances_[t.target] = target_balance + t.amount;
    }

    // Only for after the threads have finished.
    const std::vector<long>& balances() const { return balances_; }

private:
    std::vector<long> balances_;
    // One lock for each account, so that transfers between different
    // accounts can run at the same time. A mutex can't be moved, so the
    // vector is made at its full size and never grows.
    std::vector<std::mutex> locks_;
};

int main() {
    std::vector<std::string> lines;
    for (std::string line; std::getline(std::cin, line);) lines.push_back(line);
    auto first = numbers(lines.empty() ? "" : lines[0]);
    if (!first || first->size() != 1 || (*first)[0] < 1 || (*first)[0] > 16) {
        std::cout << "error: line 1 must be the number of workers, from 1 to 16\n";
        return 0;
    }
    size_t workers = (*first)[0];
    auto balances = numbers(lines.size() > 1 ? lines[1] : "");
    if (!balances || balances->empty()) {
        std::cout << "error: line 2 must be the starting balances\n";
        return 0;
    }
    size_t accounts = balances->size();

    std::vector<Transfer> transfers;
    for (size_t i = 2; i < lines.size(); i++) {
        auto words = numbers(lines[i]);
        if (words && words->empty()) continue;
        if (!words || words->size() != 3) {
            std::cout << "error: line " << i + 1
                      << ": a transfer is three numbers: from, to, and amount\n";
            continue;
        }
        size_t source = (*words)[0], target = (*words)[1];
        if (source >= accounts || target >= accounts) {
            std::cout << "error: line " << i + 1 << ": no account "
                      << (source >= accounts ? source : target) << "\n";
            continue;
        }
        transfers.push_back({source, target, (*words)[2]});
    }

    // Worker i applies transfers i, i + workers, i + 2 * workers, and so on.
    // The jthreads join when the vector is destroyed, at the end of the block.
    Bank bank(*balances);
    {
        std::vector<std::jthread> threads;
        for (size_t i = 0; i < workers; i++) {
            threads.emplace_back([&bank, &transfers, i, workers] {
                for (size_t j = i; j < transfers.size(); j += workers) bank.transfer(transfers[j]);
            });
        }
    }

    std::cout << plural(workers, "worker") << ", " << plural(accounts, "account") << ", "
              << plural(transfers.size(), "transfer") << "\n";
    for (size_t i = 0; i < workers; i++) {
        size_t share = 0;
        long moved = 0;
        for (size_t j = i; j < transfers.size(); j += workers) {
            share++;
            moved += transfers[j].amount;
        }
        std::cout << "worker " << i << ": " << plural(share, "transfer") << ", " << moved
                  << " moved\n";
    }
    long total = 0;
    for (size_t i = 0; i < accounts; i++) {
        std::cout << "account " << i << ": " << bank.balances()[i] << "\n";
        total += bank.balances()[i];
    }
    std::cout << "total: " << total << "\n";
}
