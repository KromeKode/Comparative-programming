import re
import sys
from collections import Counter
from concurrent.futures import ProcessPoolExecutor

WORD = re.compile(r"[A-Za-z]+")


def plural(count: int, noun: str) -> str:
    return f"{count} {noun}" if count == 1 else f"{count} {noun}s"


# Runs in a worker process. The lines arrive as a copy, and the Counter goes
# back as a copy, so the workers share nothing.
def count_words(lines: list[str]) -> Counter[str]:
    counts: Counter[str] = Counter()
    for line in lines:
        counts.update(word.lower() for word in WORD.findall(line))
    return counts


def main() -> None:
    lines = sys.stdin.read().splitlines()
    first = lines[0].split() if lines else []
    if len(first) != 2 or not all(part.isascii() and part.isdigit() for part in first):
        print("error: the first line must be two numbers, workers and top")
        return
    workers, top = int(first[0]), int(first[1])
    if not 1 <= workers <= 16:
        print("error: workers must be from 1 to 16")
        return

    text = lines[1:]
    # Each worker gets len(text) // workers lines, and the first few get one
    # more, so that every line is counted once.
    size, extra = divmod(len(text), workers)
    chunks, start = [], 0
    for i in range(workers):
        end = start + size + (1 if i < extra else 0)
        chunks.append(text[start:end])
        start = end

    # Counting is work for the processor, and Python's threads take turns
    # running Python code, so each worker is a process. map keeps the results
    # in the order of the chunks, whichever worker finishes first.
    with ProcessPoolExecutor(max_workers=workers) as pool:
        counts = list(pool.map(count_words, chunks))

    print(f"{plural(workers, 'worker')}, {plural(len(text), 'line')}")
    total: Counter[str] = Counter()
    for i, (chunk, table) in enumerate(zip(chunks, counts, strict=True)):
        words = plural(table.total(), "word")
        print(f"worker {i}: {plural(len(chunk), 'line')}, {words}")
        total += table
    print(f"{plural(total.total(), 'word')}, {len(total)} different")
    ranked = sorted(total.items(), key=lambda item: (-item[1], item[0]))
    for word, count in ranked[:top]:
        print(f"{word}: {count}")


# A worker process can start by importing this file, as it does on macOS and
# Windows, and then main must not run again.
if __name__ == "__main__":
    main()
