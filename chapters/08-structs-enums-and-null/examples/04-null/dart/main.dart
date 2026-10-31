// String? may hold null; plain String never does, and the compiler checks it.
class User(final String name, {final String? nickname});

void main() {
  final users = [
    User('ada', nickname: 'Countess'),
    User('alan'),
    User('grace', nickname: 'Amazing Grace'),
  ];

  // firstOrNull gives null when nothing matches.
  User? find(String name) =>
      users.where((user) => user.name == name).firstOrNull;

  // a ?? b is b only when a is null.
  for (final user in users) {
    print('${user.name} is shown as ${user.nickname ?? user.name}');
  }

  // a?.b stops and gives null when a is null.
  final lengths = users.map((user) => user.nickname?.length ?? 0);
  print('nickname lengths: ${lengths.join(' ')}');

  for (final name in ['alan', 'linus']) {
    print('find "$name": ${find(name)?.name ?? 'nobody'}');
  }

  for (final name in ['ada', 'alan', 'linus']) {
    print('nickname of "$name": ${find(name)?.nickname ?? 'none'}');
  }
}
