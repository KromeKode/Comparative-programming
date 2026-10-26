def describe(code: int) -> str:
    match code:  # Python 3.10 and later
        case 200:
            return "ok"
        case 301 | 302 | 307 | 308:
            return "redirect"
        # Cases are tried from top to bottom, so 404 must come before the range.
        case 404:
            return "not found"
        case _ if 400 <= code <= 499:
            return "client error"
        case _ if 500 <= code <= 599:
            return "server error"
        case _:
            return "unknown"


def main() -> None:
    for code in [200, 301, 308, 404, 418, 500, 503, 99]:
        print(f"{code}: {describe(code)}")


if __name__ == "__main__":
    main()
