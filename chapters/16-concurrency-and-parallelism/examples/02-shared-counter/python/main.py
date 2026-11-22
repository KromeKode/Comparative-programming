import threading

THREADS = 4
TIMES = 100_000

locked_count = 0
lock = threading.Lock()


def add_with_lock() -> None:
    global locked_count
    for _ in range(TIMES):
        # with lock: holds the lock for the block, and releases it on the way
        # out, even if the block raises.
        with lock:
            locked_count += 1


def main() -> None:
    print(f"{THREADS} threads add 1, {TIMES} times each")
    threads = [threading.Thread(target=add_with_lock) for _ in range(THREADS)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    print(f"with a lock: {locked_count}")
    # Python has no atomic integers. Even with the GIL, which lets only one
    # thread run Python code at a time, += is several steps, and a thread
    # can be paused between them.


if __name__ == "__main__":
    main()
