import 'dart:io';

typedef Grid = List<List<String>>;

// List.of(grid) would copy only the outer list, and the copy would share its
// rows with the grid. Copy each row too.
Grid copyGrid(Grid grid) => [for (final row in grid) List.of(row)];

// List.filled(height, row) would put one row in every slot, so make each row.
Grid makeGrid(int width, int height, String c) =>
    List.generate(height, (_) => List.filled(width, c));

final digits = RegExp(r'^[0-9]+$');

int? parseNumber(String text) {
  if (!digits.hasMatch(text)) {
    print('error: "$text" is not a number');
    return null;
  }
  // tryParse gives null for a number too big for an int; that is outside
  // any grid, so use a value that is too.
  return int.tryParse(text) ?? -1;
}

String? oneCharacter(String text) {
  if (text.runes.length != 1) {
    print('error: "$text" is not one character');
    return null;
  }
  return text;
}

bool takes(String command, int want, int got) {
  if (got != want) {
    final plural = want == 1 ? '' : 's';
    print('error: $command takes $want value$plural, got $got');
  }
  return got == want;
}

List<String> fields(String line) =>
    line.split(RegExp(r'\s+')).where((part) => part.isNotEmpty).toList();

void main() {
  final first = stdin.readLineSync();
  if (first == null) return;
  final sizes = fields(first).map(int.parse).toList();
  final (width, height) = (sizes[0], sizes[1]);
  var grid = makeGrid(width, height, '.');
  Grid? saved;

  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = fields(line);
    if (parts.isEmpty) continue;
    final command = parts[0];
    final values = parts.sublist(1);
    switch (command) {
      case 'paint':
        if (!takes(command, 3, values.length)) continue;
        final x = parseNumber(values[0]);
        if (x == null) continue;
        final y = parseNumber(values[1]);
        if (y == null) continue;
        if (x < 1 || x > width || y < 1 || y > height) {
          final where = '(${values[0]}, ${values[1]})';
          print('error: $where is outside the $width by $height grid');
          continue;
        }
        final c = oneCharacter(values[2]);
        if (c != null) grid[y - 1][x - 1] = c;
      case 'fill':
        if (!takes(command, 1, values.length)) continue;
        final c = oneCharacter(values[0]);
        if (c != null) grid = makeGrid(width, height, c);
      case 'save':
        if (takes(command, 0, values.length)) saved = copyGrid(grid);
      case 'restore':
        if (!takes(command, 0, values.length)) continue;
        final snapshot = saved;
        if (snapshot == null) {
          print('error: nothing saved');
        } else {
          // grid = snapshot would share one grid, and the next paint would
          // change the snapshot as well.
          grid = copyGrid(snapshot);
        }
      case 'show':
        if (takes(command, 0, values.length)) {
          for (final row in grid) {
            print(row.join());
          }
          print('');
        }
      default:
        print('error: unknown command "$command"');
    }
  }
}
