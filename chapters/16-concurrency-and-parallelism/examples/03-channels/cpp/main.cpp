#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

// The standard library has no channel, but it has the parts: a queue, a
// mutex that guards it, and condition variables, which let a thread sleep
// until another one says that something has changed.
template <typename T>
class Channel {
public:
    explicit Channel(std::size_t capacity) : capacity_(capacity) {}

    void send(T item) {
        std::unique_lock lock(mutex_);
        // wait unlocks the mutex while it sleeps, and returns, locked, once
        // the condition is true.
        not_full_.wait(lock, [&] { return items_.size() < capacity_; });
        items_.push(std::move(item));
        not_empty_.notify_one();
    }

    // Returns nothing once the channel is closed and empty.
    std::optional<T> receive() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [&] { return !items_.empty() || closed_; });
        if (items_.empty()) return std::nullopt;
        T item = std::move(items_.front());
        items_.pop();
        not_full_.notify_one();
        return item;
    }

    void close() {
        std::lock_guard lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();  // wake every waiting receiver
    }

private:
    std::mutex mutex_;
    std::condition_variable not_empty_, not_full_;
    std::queue<T> items_;
    std::size_t capacity_;
    bool closed_ = false;
};

constexpr long count = 1000;
constexpr int workers = 3;

int main() {
    Channel<long> jobs(16), results(16);
    std::atomic<int> workers_left = workers;

    std::jthread producer([&] {
        for (long n = 1; n <= count; n++) jobs.send(n);
        jobs.close();
    });

    std::vector<std::jthread> pool;
    for (int i = 0; i < workers; i++) {
        pool.emplace_back([&] {
            while (auto n = jobs.receive()) results.send(*n * *n);
            // The last worker to finish closes the results.
            if (--workers_left == 0) results.close();
        });
    }

    long received = 0, sum = 0;
    while (auto square = results.receive()) {
        received++;
        sum += *square;
    }
    std::cout << workers << " workers squared 1 to " << count << "\n";
    std::cout << "results: " << received << "\n";
    std::cout << "sum: " << sum << "\n";
}
