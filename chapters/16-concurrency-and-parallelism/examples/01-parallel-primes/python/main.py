from concurrent.futures import ProcessPoolExecutor

LIMIT = 200_000
WORKERS = 4


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    d = 2
    while d * d <= n:
        if n % d == 0:
            return False
        d += 1
    return True


def count_primes(start: int, end: int) -> int:
    return sum(1 for n in range(start, end) if is_prime(n))


def main() -> None:
    print(f"counting primes below {LIMIT} with {WORKERS} workers")
    starts = [LIMIT * i // WORKERS for i in range(WORKERS)]
    ends = [LIMIT * (i + 1) // WORKERS for i in range(WORKERS)]

    # Python's threads take turns running Python code, so for work like this
    # each worker is a separate process. map sends the arguments to the
    # processes and gives back the results in order.
    with ProcessPoolExecutor(max_workers=WORKERS) as pool:
        counts = list(pool.map(count_primes, starts, ends))

    for i, count in enumerate(counts):
        print(f"worker {i}: [{starts[i]}, {ends[i]}) has {count} primes")
    print(f"total: {sum(counts)} primes")


# A worker process can start by importing this file, as it does on macOS and
# Windows, and then main must not run again.
if __name__ == "__main__":
    main()
