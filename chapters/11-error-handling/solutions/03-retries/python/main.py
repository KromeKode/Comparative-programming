import sys

MAX_ATTEMPTS = 3


class DownloadError(Exception):
    """The base class. Each kind says whether to try again or stop everything."""

    retry = False
    fatal = False


class TimedOut(DownloadError):
    retry = True

    def __init__(self) -> None:
        super().__init__("timed out")


class ServerBusy(DownloadError):
    retry = True

    def __init__(self) -> None:
        super().__init__("server busy")


class NotFound(DownloadError):
    def __init__(self) -> None:
        super().__init__("not found")


class LoginRefused(DownloadError):
    fatal = True

    def __init__(self) -> None:
        super().__init__("login refused")


class GaveUp(DownloadError):
    def __init__(self, attempts: int, last: DownloadError) -> None:
        super().__init__(f"gave up after {attempts} attempts: {last}")


ERRORS = {
    "timeout": TimedOut,
    "busy": ServerBusy,
    "missing": NotFound,
    "auth": LoginRefused,
}


def download(answers: list[str], attempt: int) -> None:
    """The pretend server: raise the error for this attempt's answer, if any."""
    answer = answers[min(attempt, len(answers)) - 1]
    if answer != "ok":
        raise ERRORS[answer]()


def download_with_retries(name: str, answers: list[str]) -> int:
    """Return the number of attempts it took, or raise the error that stopped it."""
    wait = 1
    for attempt in range(1, MAX_ATTEMPTS + 1):
        try:
            download(answers, attempt)
            return attempt
        except DownloadError as error:
            if not error.retry:
                raise
            if attempt == MAX_ATTEMPTS:
                raise GaveUp(attempt, error) from error
            print(f"{name}: {error}, retrying in {wait}s")
            wait *= 2
    raise AssertionError("the loop always returns or raises")


def main() -> None:
    jobs = [line.split() for line in sys.stdin.read().splitlines() if line.strip()]
    downloaded = skipped = failed = not_tried = 0
    for index, (name, *answers) in enumerate(jobs):
        try:
            attempts = download_with_retries(name, answers)
        except GaveUp as error:
            print(f"{name}: failed: {error}")
            failed += 1
        except DownloadError as error:
            if error.fatal:
                print(f"{name}: stopped: {error}")
                failed += 1
                not_tried = len(jobs) - index - 1
                break
            print(f"{name}: skipped: {error}")
            skipped += 1
        else:
            print(f"{name}: ok after {attempts} attempt{'' if attempts == 1 else 's'}")
            downloaded += 1
    print(
        f"downloaded {downloaded}, skipped {skipped}, failed {failed}, "
        f"not tried {not_tried}"
    )


if __name__ == "__main__":
    main()
