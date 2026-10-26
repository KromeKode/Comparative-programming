const maxPlayers = 4; // compile-time constant

void main() {
  var count = 0; // var: type inferred (int), can be reassigned
  final birthYear = 1815; // final: assigned exactly once
  const pi = 3.14159;
  final bool ready = true; // types can also be written out

  count += 3;

  print('count = $count');
  print('birth year = $birthYear');
  print('pi = $pi');
  print('max players = $maxPlayers');
  print('ready = $ready');
}
