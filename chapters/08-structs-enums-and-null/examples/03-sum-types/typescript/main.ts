// A discriminated union: the kind field tells the compiler which variant it has.
type Payment =
  | { kind: "cash" }
  | { kind: "card"; last4: string }
  | { kind: "voucher"; code: string; cents: number };

const money = (cents: number): string =>
  `${Math.floor(cents / 100)}.${String(cents % 100).padStart(2, "0")}`;

function describe(payment: Payment, total: number): string {
  switch (payment.kind) {
    case "cash":
      return `cash: pay ${money(total)} in cash`;
    case "card": // in this case, TypeScript knows payment has last4
      return `card ending ${payment.last4}: charge ${money(total)} to the card`;
    case "voucher": {
      const { code, cents } = payment;
      if (cents >= total) {
        return `voucher ${code} (${money(cents)}): covers the order`;
      }
      return `voucher ${code} (${money(cents)}): ${money(total - cents)} left to pay`;
    }
    default: {
      // If a new kind is added and not handled above, this line stops compiling.
      const unhandled: never = payment;
      throw new Error(`unknown payment: ${JSON.stringify(unhandled)}`);
    }
  }
}

const payments: Payment[] = [
  { kind: "cash" },
  { kind: "card", last4: "4242" },
  { kind: "voucher", code: "SPRING", cents: 500 },
  { kind: "voucher", code: "BIG", cents: 2000 },
];
const total = 1250;
console.log(`order total: ${money(total)}`);
for (const payment of payments) {
  console.log(describe(payment, total));
}
