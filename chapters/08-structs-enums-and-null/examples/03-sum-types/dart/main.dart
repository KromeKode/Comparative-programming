// A sealed class can only be extended in this file, so the compiler knows every
// variant.
sealed class Payment {}

class Cash extends Payment {}

class Card(final String last4) extends Payment;

class Voucher(final String code, final int cents) extends Payment;

String money(int cents) =>
    '${cents ~/ 100}.${(cents % 100).toString().padLeft(2, '0')}';

// A switch expression on a sealed type must cover every subclass.
String describe(Payment payment, int total) => switch (payment) {
  Cash() => 'cash: pay ${money(total)} in cash',
  Card(:final last4) =>
    'card ending $last4: charge ${money(total)} to the card',
  Voucher(:final code, :final cents) when cents >= total =>
    'voucher $code (${money(cents)}): covers the order',
  Voucher(:final code, :final cents) =>
    'voucher $code (${money(cents)}): ${money(total - cents)} left to pay',
};

void main() {
  const total = 1250;
  final payments = [
    Cash(),
    Card('4242'),
    Voucher('SPRING', 500),
    Voucher('BIG', 2000),
  ];
  print('order total: ${money(total)}');
  for (final payment in payments) {
    print(describe(payment, total));
  }
}
