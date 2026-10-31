void main() {
  final inputs = ['42', '4x2', '', '99999999999999999999'];

  // int.parse throws a FormatException when it cannot parse.
  print('int.parse:');
  for (final text in inputs) {
    try {
      print('  "$text" -> ${int.parse(text)}');
    } on FormatException catch (error) {
      print('  "$text" -> FormatException: ${error.message}');
    }
  }

  // int.tryParse returns null instead, and the type int? makes you check.
  print('int.tryParse:');
  for (final text in inputs) {
    final value = int.tryParse(text);
    print('  "$text" -> ${value ?? 'null, so use a default: 0'}');
  }
}
