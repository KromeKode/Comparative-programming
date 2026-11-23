// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. package:args reads the options and writes the help from them; it throws a
// FormatException when the command line is wrong, and the exit status is ours.
import 'dart:io';

import 'package:args/args.dart';

const usage = 'usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...';
const units = ['c', 'f', 'k'];

double toCelsius(double t, String unit) => switch (unit) {
  'f' => (t - 32) * 5 / 9,
  'k' => t - 273.15,
  _ => t,
};

double fromCelsius(double t, String unit) => switch (unit) {
  'f' => t * 9 / 5 + 32,
  'k' => t + 273.15,
  _ => t,
};

/// Prints a message and the usage line to stderr, and exits with status 2.
Never usageError(String message) {
  stderr.writeln('temp: $message\n$usage');
  exit(2);
}

/// A number followed by c, f, or k, or null.
(double, String)? parseTemperature(String text) {
  if (text.isEmpty) return null;
  final unit = text.substring(text.length - 1);
  final value = double.tryParse(text.substring(0, text.length - 1));
  if (!units.contains(unit) || value == null || !value.isFinite) return null;
  return (value, unit);
}

void main(List<String> arguments) {
  final parser = ArgParser()
    ..addFlag(
      'help',
      abbr: 'h',
      negatable: false,
      help: 'show this help and exit',
    )
    ..addOption(
      'to',
      abbr: 't',
      allowed: units,
      defaultsTo: 'c',
      valueHelp: 'UNIT',
      help: 'the unit to convert to',
    )
    ..addOption(
      'digits',
      abbr: 'd',
      defaultsTo: '1',
      valueHelp: 'DIGITS',
      help: 'digits after the point, 0 to 9',
    );

  final ArgResults results;
  try {
    results = parser.parse(arguments);
  } on FormatException catch (error) {
    usageError(error.message);
  }
  if (results.flag('help')) {
    print('$usage\n\nConverts temperatures, such as 21.5c, -40f, or 300k.\n');
    print(parser.usage);
    exit(0);
  }
  final to = results.option('to')!;
  final digitsText = results.option('digits')!;
  final digits = int.tryParse(digitsText);
  // '$digits' is digitsText only when it was a plain whole number, such as 3.
  if (digits == null || digits < 0 || digits > 9 || '$digits' != digitsText) {
    usageError('digits must be 0 to 9, not $digitsText');
  }
  if (results.rest.isEmpty) usageError('no temperatures');

  // Check every temperature before printing any, so that a mistake prints nothing.
  final temperatures = <(String, double, String)>[];
  for (final text in results.rest) {
    final temperature = parseTemperature(text);
    if (temperature == null) {
      stderr.writeln('temp: not a temperature: $text');
      exit(2);
    }
    final (value, unit) = temperature;
    temperatures.add((text, value, unit));
  }

  for (final (text, value, unit) in temperatures) {
    final converted = fromCelsius(toCelsius(value, unit), to);
    print('$text = ${converted.toStringAsFixed(digits)}$to');
  }
}
