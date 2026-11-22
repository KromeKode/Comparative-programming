import sys

# Python looks for textstats.py in the folder of the script it is running, then
# on the rest of sys.path, and runs it once, the first time it is imported.
import textstats


def main() -> None:
    text = sys.stdin.read()
    print(f"textstats {textstats.VERSION}")
    print(f"words: {textstats.word_count(text)}")
    print(f"longest: {textstats.longest_word(text)}")


if __name__ == "__main__":
    main()
