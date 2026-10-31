// An enhanced enum can carry fields, set by its constructor.
enum Light {
  red(30),
  green(25),
  yellow(5);

  const Light(this.seconds);
  final int seconds;

  Light get next => values[(index + 1) % values.length];

  // A switch expression must cover every value, or it does not compile.
  String get meaning => switch (this) {
    Light.red => 'stop',
    Light.green => 'go',
    Light.yellow => 'slow down',
  };
}

void showParse(String text) {
  // asNameMap() maps each name to its value; a missing name gives null.
  final light = Light.values.asNameMap()[text];
  print('parse "$text": ${light?.name ?? 'not a light'}');
}

void main() {
  print('lights: ${Light.values.map((light) => light.name).join(' ')}');
  print('yellow is number ${Light.yellow.index}');

  var light = Light.red;
  for (final _ in Light.values) {
    print('after ${light.name}: ${light.next.name}');
    light = light.next;
  }
  print(
    'red means ${Light.red.meaning}, green means ${Light.green.meaning}, '
    'yellow means ${Light.yellow.meaning}',
  );

  final total = Light.values.fold(0, (sum, each) => sum + each.seconds);
  print('one cycle: $total seconds');

  showParse('green');
  showParse('blue');
}
