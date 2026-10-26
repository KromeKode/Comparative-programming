// Dart has no binary literals, so build each flag with a shift.
const read = 1 << 0; // 001
const write = 1 << 1; // 010
const exec = 1 << 2; // 100

String bits8(int n) => n.toRadixString(2).padLeft(8, '0');

int countOnes(int n) {
  var count = 0;
  for (var rest = n; rest != 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

void main() {
  var perms = read | write; // turn bits on with |
  print('read | write: ${bits8(perms)}');
  // & binds tighter than == in Dart, so no parentheses are needed.
  print('has write? ${perms & write != 0 ? 'yes' : 'no'}');
  print('has exec? ${perms & exec != 0 ? 'yes' : 'no'}');
  perms &= ~write; // turn a bit off: AND with every bit except write
  print('clear write: ${bits8(perms)}');
  perms ^= exec; // flip a bit with ^
  print('toggle exec: ${bits8(perms)}');
  print('5 << 2 = ${5 << 2}');
  print('-16 >> 2 = ${-16 >> 2}');
  print('set bits in 10110: ${countOnes(int.parse('10110', radix: 2))}');
}
