from types import TracebackType


class File:
    """A pretend file. `with` calls __enter__ at the start and __exit__ at the end."""

    def __init__(self, name: str, fails: bool = False) -> None:
        if fails:
            raise OSError(f"cannot open {name}")
        print(f"  open {name}")
        self.name = name

    def __enter__(self) -> "File":
        return self

    def __exit__(
        self,
        kind: type[BaseException] | None,
        error: BaseException | None,
        traceback: TracebackType | None,
    ) -> None:
        print(f"  close {self.name}")


def run_job(output_fails: bool, copy_fails: bool) -> None:
    # However the block ends, each file that opened is closed, in the
    # reverse order.
    with File("input"), File("output", output_fails):
        if copy_fails:
            raise OSError("disk full")
        print("  copy input to output")


def main() -> None:
    jobs = [
        ("everything works", False, False),
        ("opening output fails", True, False),
        ("copying fails", False, True),
    ]
    for number, (title, output_fails, copy_fails) in enumerate(jobs, start=1):
        print(f"job {number}: {title}")
        try:
            run_job(output_fails, copy_fails)
            print(f"  job {number} done")
        except OSError as error:
            print(f"  job {number} failed: {error}")


if __name__ == "__main__":
    main()
