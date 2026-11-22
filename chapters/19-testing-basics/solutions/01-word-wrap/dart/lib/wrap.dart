/// Wraps [text] into lines of at most [width] characters, breaking only at spaces.
List<String> wrap(String text, int width) {
  final lines = <String>[];
  var line = '';
  for (final word in text.split(' ')) {
    if (word.isEmpty) {
      continue;
    }
    if (line.isEmpty) {
      line = word;
    } else if (line.length + 1 + word.length <= width) {
      line += ' $word';
    } else {
      lines.add(line);
      line = word;
    }
  }
  if (line.isNotEmpty) {
    lines.add(line);
  }
  return lines;
}
