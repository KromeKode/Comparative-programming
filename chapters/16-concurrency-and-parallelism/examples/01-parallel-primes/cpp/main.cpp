#include <future>
#include <iostream>
#include <vector>

constexpr long limit = 200000;
constexpr int workers = 4;

bool is_prime(long n) {
    if (n < 2) return false;
    for (long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

long count_primes(long start, long end) {
    long count = 0;
    for (long n = start; n < end; n++) {
        if (is_prime(n)) count++;
    }
    return count;
}

int main() {
    std::cout << "counting primes below " << limit << " with " << workers << " workers\n";

    // std::async with std::launch::async runs the function on a new thread
    // and returns a future, which will hold its result.
    std::vector<std::future<long>> results;
    for (int i = 0; i < workers; i++) {
        results.push_back(std::async(std::launch::async, count_primes, limit * i / workers,
                                     limit * (i + 1) / workers));
    }

    // get() waits for the thread to finish and returns what it returned.
    long total = 0;
    for (int i = 0; i < workers; i++) {
        long count = results[i].get();
        std::cout << "worker " << i << ": [" << limit * i / workers << ", "
                  << limit * (i + 1) / workers << ") has " << count << " primes\n";
        total += count;
    }
    std::cout << "total: " << total << " primes\n";
}
