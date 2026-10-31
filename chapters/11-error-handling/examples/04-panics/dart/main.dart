class User {
  final String name;
  User(this.name);
}

final numbers = [10, 20, 30];
final users = {1: User('ada')};
const index = 5, zero = 0;

User? findUser(int id) => users[id];

// Run action and report what it returned or what it threw.
void attempt(String label, Object? Function() action) {
  try {
    print('$label: ${action()}');
  } catch (error) {
    // A RangeError's message starts with its type, but a TypeError's does not.
    final type = error is TypeError ? 'TypeError: ' : '';
    print('$label: $type$error');
  }
}

void main() {
  attempt('numbers[5]', () => numbers[index]);
  attempt('10 ~/ zero', () => 10 ~/ zero);
  attempt('10 / zero', () => 10 / zero);
  attempt('findUser(7)!.name', () => findUser(7)!.name);
  attempt('assert(numbers.length == 4)', () {
    // dart run skips asserts unless you pass --enable-asserts.
    assert(numbers.length == 4, 'expected 4 numbers');
    return 'passed';
  });
  // The checked version returns null instead.
  attempt('numbers.elementAtOrNull(5)', () => numbers.elementAtOrNull(index));
}
