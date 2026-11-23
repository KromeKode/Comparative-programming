# Counts the words in its input, and prints the ten most common. It is slow on a long
# text, and section 20.7 runs it under a profiler to find out why.
import sys


def split_words(text: str) -> list[str]:
    """Gives the words of the text: runs of the letters a to z, in lowercase."""
    words = []
    word = ""
    for c in text.lower():
        if "a" <= c <= "z":
            word += c
        elif word:
            words.append(word)
            word = ""
    if word:
        words.append(word)
    return words


def find(counts: list[tuple[str, int]], word: str) -> int:
    """Gives the index of the word's count, or -1 if it has none."""
    for i, (w, _) in enumerate(counts):
        if w == word:
            return i
    return -1


def count_words(words: list[str]) -> list[tuple[str, int]]:
    counts: list[tuple[str, int]] = []
    for word in words:
        i = find(counts, word)
        if i == -1:
            counts.append((word, 1))
        else:
            counts[i] = (word, counts[i][1] + 1)
    return counts


def main() -> None:
    counts = count_words(split_words(sys.stdin.read()))
    counts.sort(key=lambda pair: (-pair[1], pair[0]))
    for word, n in counts[:10]:
        print(n, word)


if __name__ == "__main__":
    main()
