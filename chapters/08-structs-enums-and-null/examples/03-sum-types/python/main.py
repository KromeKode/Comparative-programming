from dataclasses import dataclass


@dataclass(frozen=True)
class Cash:
    pass


@dataclass(frozen=True)
class Card:
    last4: str


@dataclass(frozen=True)
class Voucher:
    code: str
    cents: int


# A union of classes: a Payment is exactly one of these three.
type Payment = Cash | Card | Voucher


def money(cents: int) -> str:
    return f"{cents // 100}.{cents % 100:02}"


def describe(payment: Payment, total: int) -> str:
    # Each case is a pattern. Card(last4) checks the type and binds its field.
    match payment:
        case Cash():
            return f"cash: pay {money(total)} in cash"
        case Card(last4):
            return f"card ending {last4}: charge {money(total)} to the card"
        case Voucher(code, cents) if cents >= total:
            return f"voucher {code} ({money(cents)}): covers the order"
        case Voucher(code, cents):
            left = money(total - cents)
            return f"voucher {code} ({money(cents)}): {left} left to pay"


def main() -> None:
    total = 1250
    payments: list[Payment] = [
        Cash(),
        Card("4242"),
        Voucher("SPRING", 500),
        Voucher("BIG", 2000),
    ]
    print(f"order total: {money(total)}")
    for payment in payments:
        print(describe(payment, total))


if __name__ == "__main__":
    main()
