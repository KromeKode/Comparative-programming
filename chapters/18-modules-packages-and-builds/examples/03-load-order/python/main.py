import settings


def main() -> None:
    print("main: start")
    print(f"main: the greeting is {settings.GREETING}")
    print(f"main: the greeting is still {settings.GREETING}")


if __name__ == "__main__":
    main()
