def wrap(text: str, width: int) -> list[str]:
    """Wraps text into lines of at most width characters, breaking only at spaces."""
    lines: list[str] = []
    line = ""
    for word in text.split(" "):
        if word == "":
            continue
        if line == "":
            line = word
        elif len(line) + 1 + len(word) <= width:
            line += " " + word
        else:
            lines.append(line)
            line = word
    if line != "":
        lines.append(line)
    return lines
