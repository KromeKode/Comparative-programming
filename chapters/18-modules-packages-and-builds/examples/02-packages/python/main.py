import sys

import textstats


def main() -> None:
    text = sys.stdin.read()
    print(f"textstats {textstats.VERSION}")
    print(f"words: {textstats.word_count(text)}")
    print(f"longest: {textstats.longest_word(text)}")


if __name__ == "__main__":
    main()
