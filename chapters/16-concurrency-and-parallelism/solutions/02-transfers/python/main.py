import sys
import threading
import time
from dataclasses import dataclass


@dataclass(frozen=True)
class Transfer:
    source: int
    target: int
    amount: int


def plural(count: int, noun: str) -> str:
    return f"{count} {noun}" if count == 1 else f"{count} {noun}s"


def is_number(word: str) -> bool:
    return word.isascii() and word.isdigit()


class Bank:
    def __init__(self, balances: list[int]) -> None:
        self.balances = balances
        # One lock for each account, so that transfers between different
        # accounts can run at the same time.
        self.locks = [threading.Lock() for _ in balances]

    def transfer(self, transfer: Transfer) -> None:
        source, target = transfer.source, transfer.target
        if source == target:
            return  # one lock taken twice would wait for itself forever
        # Every worker takes the two locks in the same order, lower account
        # first, so no two workers can each hold the lock the other wants.
        first, second = sorted((source, target))
        with self.locks[first], self.locks[second]:
            source_balance = self.balances[source]
            target_balance = self.balances[target]
            # Gives up the GIL, so another thread runs between the read and
            # the write, as it could while a real bank waits on its database.
            time.sleep(0)
            self.balances[source] = source_balance - transfer.amount
            self.balances[target] = target_balance + transfer.amount


def parse(lines: list[str]) -> tuple[int, list[int], list[Transfer]]:
    """Returns the workers, balances, and transfers, and prints the errors."""
    first = lines[0].split() if lines else []
    if len(first) != 1 or not is_number(first[0]) or not 1 <= int(first[0]) <= 16:
        raise ValueError("line 1 must be the number of workers, from 1 to 16")
    second = lines[1].split() if len(lines) > 1 else []
    if not second or not all(is_number(word) for word in second):
        raise ValueError("line 2 must be the starting balances")
    balances = [int(word) for word in second]
    transfers = []
    for number, line in enumerate(lines[2:], start=3):
        words = line.split()
        if not words:
            continue
        if len(words) != 3 or not all(is_number(word) for word in words):
            print(
                f"error: line {number}: a transfer is three numbers: from, to, and amount"
            )
            continue
        source, target, amount = map(int, words)
        missing = [account for account in (source, target) if account >= len(balances)]
        if missing:
            print(f"error: line {number}: no account {missing[0]}")
            continue
        transfers.append(Transfer(source, target, amount))
    return int(first[0]), balances, transfers


def main() -> None:
    try:
        workers, balances, transfers = parse(sys.stdin.read().splitlines())
    except ValueError as error:
        print(f"error: {error}")
        return
    bank = Bank(balances)

    def work(share: list[Transfer]) -> None:
        for transfer in share:
            bank.transfer(transfer)

    # Worker i gets transfers i, i + workers, i + 2 * workers, and so on.
    shares = [transfers[i::workers] for i in range(workers)]
    threads = [threading.Thread(target=work, args=(share,)) for share in shares]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    print(
        f"{plural(workers, 'worker')}, {plural(len(balances), 'account')},"
        f" {plural(len(transfers), 'transfer')}"
    )
    for i, share in enumerate(shares):
        moved = sum(t.amount for t in share)
        print(f"worker {i}: {plural(len(share), 'transfer')}, {moved} moved")
    for account, balance in enumerate(bank.balances):
        print(f"account {account}: {balance}")
    print(f"total: {sum(bank.balances)}")


if __name__ == "__main__":
    main()
