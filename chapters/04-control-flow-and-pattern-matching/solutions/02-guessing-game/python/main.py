import sys


def guesses_text(count: int) -> str:
    return "1 guess" if count == 1 else f"{count} guesses"


def main() -> None:
    lines = iter(sys.stdin)
    secret = int(next(lines))

    guesses = 0
    for line in lines:
        text = line.strip()
        if not text:
            continue  # a blank line
        try:
            guess = int(text)
        except ValueError:
            print(f"{text}: not a number")
            continue
        guesses += 1
        if guess < secret:
            print(f"{guess}: too low")
        elif guess > secret:
            print(f"{guess}: too high")
        else:
            print(f"{guess}: correct in {guesses_text(guesses)}")
            break
    else:  # the loop ran out of lines without a break
        print(f"gave up after {guesses_text(guesses)}; the number was {secret}")


if __name__ == "__main__":
    main()
