import 'dart:io';

// Clockwise order, so turning right adds 1 and turning left subtracts 1.
enum Direction {
  north(0, 1),
  east(1, 0),
  south(0, -1),
  west(-1, 0);

  const Direction(this.dx, this.dy);
  final int dx;
  final int dy;

  // Dart's % never gives a negative result for a positive right side.
  Direction turn(int steps) => values[(index + steps) % values.length];
}

class Robot {
  int x = 0;
  int y = 0;
  Direction facing = Direction.north;

  void run(String command) {
    switch (command) {
      case 'L':
        facing = facing.turn(-1);
      case 'R':
        facing = facing.turn(1);
      case 'F':
        x += facing.dx;
        y += facing.dy;
    }
  }
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    if (line.isEmpty) continue;
    final bad = line.split('').indexWhere((c) => !'LRF'.contains(c));
    if (bad != -1) {
      print("error: unknown command '${line[bad]}' at position ${bad + 1}");
      continue;
    }
    final robot = Robot();
    for (final command in line.split('')) {
      robot.run(command);
    }
    print('${robot.x} ${robot.y} ${robot.facing.name}');
  }
}
