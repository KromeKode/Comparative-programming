import sys


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if not text:
            continue
        start = int(text)
        n = peak = start
        steps = 0
        while n != 1:
            n = n // 2 if n % 2 == 0 else 3 * n + 1
            peak = max(peak, n)
            steps += 1
        print(f"{start}: steps {steps}, peak {peak}")


if __name__ == "__main__":
    main()
