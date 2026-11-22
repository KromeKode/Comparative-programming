#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int threads = 4;
constexpr int times = 100000;

// Runs work on several threads at once. A std::jthread joins its thread when
// it is destroyed, so the vector's destructor waits for all of them.
void run(const std::function<void()>& work) {
    std::vector<std::jthread> workers;
    for (int i = 0; i < threads; i++) workers.emplace_back(work);
}

int main() {
    std::cout << threads << " threads add 1, " << times << " times each\n";

    // A lock_guard locks the mutex when it is made and unlocks it when it
    // goes out of scope, even if the code between throws.
    long locked_count = 0;
    std::mutex lock;
    run([&] {
        for (int i = 0; i < times; i++) {
            std::lock_guard guard(lock);
            locked_count += 1;
        }
    });
    std::cout << "with a mutex: " << locked_count << "\n";

    // ++ on a std::atomic reads, adds, and writes as one step.
    std::atomic<long> atomic_count = 0;
    run([&] {
        for (int i = 0; i < times; i++) ++atomic_count;
    });
    std::cout << "with an atomic: " << atomic_count << "\n";
}
