import queue
import threading

COUNT = 1000
WORKERS = 3


def main() -> None:
    # A Queue is safe to use from several threads. With a maxsize, put waits
    # while the queue is full.
    jobs: queue.Queue[int] = queue.Queue(maxsize=16)
    results: queue.Queue[int] = queue.Queue(maxsize=16)

    def produce() -> None:
        for n in range(1, COUNT + 1):
            jobs.put(n)
        # shutdown (Python 3.13) closes the queue: once it is empty, get
        # raises ShutDown instead of waiting.
        jobs.shutdown()

    def work() -> None:
        while True:
            try:
                n = jobs.get()
            except queue.ShutDown:
                return
            results.put(n * n)

    workers = [threading.Thread(target=work) for _ in range(WORKERS)]

    def close_results() -> None:
        for worker in workers:
            worker.join()
        results.shutdown()

    threading.Thread(target=produce).start()
    for worker in workers:
        worker.start()
    threading.Thread(target=close_results).start()

    received = total = 0
    while True:
        try:
            square = results.get()
        except queue.ShutDown:
            break
        received += 1
        total += square
    print(f"{WORKERS} workers squared 1 to {COUNT}")
    print(f"results: {received}")
    print(f"sum: {total}")


if __name__ == "__main__":
    main()
