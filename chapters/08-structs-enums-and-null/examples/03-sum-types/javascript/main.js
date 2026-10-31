// A plain object whose kind field says which variant it is.
const payments = [
  { kind: "cash" },
  { kind: "card", last4: "4242" },
  { kind: "voucher", code: "SPRING", cents: 500 },
  { kind: "voucher", code: "BIG", cents: 2000 },
];

const money = (cents) =>
  `${Math.floor(cents / 100)}.${String(cents % 100).padStart(2, "0")}`;

function describe(payment, total) {
  switch (payment.kind) {
    case "cash":
      return `cash: pay ${money(total)} in cash`;
    case "card":
      return `card ending ${payment.last4}: charge ${money(total)} to the card`;
    case "voucher": {
      const { code, cents } = payment;
      if (cents >= total) {
        return `voucher ${code} (${money(cents)}): covers the order`;
      }
      return `voucher ${code} (${money(cents)}): ${money(total - cents)} left to pay`;
    }
    default:
      throw new Error(`unknown payment kind: ${payment.kind}`);
  }
}

const total = 1250;
console.log(`order total: ${money(total)}`);
for (const payment of payments) {
  console.log(describe(payment, total));
}
