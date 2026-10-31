String yesNo(bool condition) => condition ? 'yes' : 'no';

void main() {
  const text = '  The quick  brown fox  ';

  final trimmed = text.trim();
  print('trimmed: [$trimmed]');
  print('words: ${trimmed.split(RegExp(r'\s+')).join('|')}');
  print('upper: ${trimmed.toUpperCase()}');
  print('contains "quick": ${yesNo(trimmed.contains('quick'))}');
  print('index of "brown": ${trimmed.indexOf('brown')}'); // -1 if missing
  print('replaced: ${trimmed.replaceAll('quick', 'slow')}');
  print('starts with "The": ${yesNo(trimmed.startsWith('The'))}');

  final fruits = ['banana', 'apple', 'Cherry'];
  fruits.sort(); // compares UTF-16 code units
  print('sorted: ${fruits.join(' ')}');
  fruits.sort((a, b) => a.toLowerCase().compareTo(b.toLowerCase()));
  print('sorted ignoring case: ${fruits.join(' ')}');
}
