import 'dart:io';
import 'dart:math';

sealed class Shape {}

class Circle(final double radius) extends Shape;

class Rectangle(final double width, final double height) extends Shape;

class Triangle(final double base, final double height) extends Shape;

// The switch must cover every subclass of the sealed Shape.
double area(Shape shape) => switch (shape) {
  Circle(:final radius) => pi * radius * radius,
  Rectangle(:final width, :final height) => width * height,
  Triangle(:final base, :final height) => base * height / 2,
};

const sizesNeeded = {'circle': 1, 'rectangle': 2, 'triangle': 2};

double parseSize(String text) {
  final value = double.tryParse(text);
  if (value == null) throw FormatException('"$text" is not a number');
  if (value < 0) throw FormatException('"$text" is negative');
  return value;
}

// Turns a shape name and its sizes into a Shape, or throws a FormatException.
Shape parse(String name, List<String> texts) {
  final needed = sizesNeeded[name];
  if (needed == null) throw FormatException('unknown shape "$name"');
  if (texts.length != needed) {
    final plural = needed == 1 ? '' : 's';
    throw FormatException(
      '$name needs $needed size$plural, got ${texts.length}',
    );
  }
  final sizes = texts.map(parseSize).toList();
  return switch (name) {
    'circle' => Circle(sizes[0]),
    'rectangle' => Rectangle(sizes[0], sizes[1]),
    _ => Triangle(sizes[0], sizes[1]),
  };
}

void main() {
  var total = 0.0;
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line
        .split(RegExp(r'\s+'))
        .where((w) => w.isNotEmpty)
        .toList();
    if (words.isEmpty) continue;
    try {
      final shape = parse(words[0], words.sublist(1));
      total += area(shape);
      print('${words[0]}: ${area(shape).toStringAsFixed(2)}');
    } on FormatException catch (error) {
      print('error: ${error.message}');
    }
  }
  print('total: ${total.toStringAsFixed(2)}');
}
