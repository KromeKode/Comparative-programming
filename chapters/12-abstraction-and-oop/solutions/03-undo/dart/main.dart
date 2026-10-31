import 'dart:io';

class EditError implements Exception {
  final String message;

  EditError(this.message);

  @override
  String toString() => message;
}

// An edit that knows how to do itself and how to undo itself. Code in other
// files can implement an interface class but not extend it.
abstract interface class Command {
  void apply(List<String> lines);

  void undo(List<String> lines);

  String describe();
}

void checkLine(List<String> lines, int number) {
  if (number < 1 || number > lines.length) {
    throw EditError('no line $number');
  }
}

class Add implements Command {
  final String text;

  Add(this.text);

  @override
  void apply(List<String> lines) => lines.add(text);

  @override
  void undo(List<String> lines) => lines.removeLast();

  @override
  String describe() => 'add "$text"';
}

class Delete implements Command {
  final int number;
  String _removed = '';

  Delete(this.number);

  @override
  void apply(List<String> lines) {
    checkLine(lines, number);
    _removed = lines.removeAt(number - 1);
  }

  @override
  void undo(List<String> lines) => lines.insert(number - 1, _removed);

  @override
  String describe() => 'delete line $number';
}

class Replace implements Command {
  final int number;
  final String text;
  String _old = '';

  Replace(this.number, this.text);

  @override
  void apply(List<String> lines) {
    checkLine(lines, number);
    _old = lines[number - 1];
    lines[number - 1] = text;
  }

  @override
  void undo(List<String> lines) => lines[number - 1] = _old;

  @override
  String describe() => 'replace line $number with "$text"';
}

class Editor {
  final _lines = <String>[];
  final _done = <Command>[];
  final _undone = <Command>[];

  void run(Command command) {
    command.apply(_lines); // if this throws, nothing is recorded
    _done.add(command);
    _undone.clear();
  }

  void undo() {
    if (_done.isEmpty) throw EditError('nothing to undo');
    final command = _done.removeLast();
    command.undo(_lines);
    _undone.add(command);
    print('undo: ${command.describe()}');
  }

  void redo() {
    if (_undone.isEmpty) throw EditError('nothing to redo');
    final command = _undone.removeLast();
    command.apply(_lines);
    _done.add(command);
    print('redo: ${command.describe()}');
  }

  void show() {
    if (_lines.isEmpty) print('(empty)');
    for (final (i, line) in _lines.indexed) {
      print('${i + 1}: $line');
    }
  }
}

int parseNumber(String text) =>
    int.tryParse(text) ?? (throw EditError('not a line number: $text'));

void run(Editor editor, List<String> words) {
  switch (words) {
    case ['add', ...final text]:
      editor.run(Add(text.join(' ')));
    case ['delete', final number]:
      editor.run(Delete(parseNumber(number)));
    case ['replace', final number, ...final text]:
      editor.run(Replace(parseNumber(number), text.join(' ')));
    case ['undo']:
      editor.undo();
    case ['redo']:
      editor.redo();
    case ['show']:
      editor.show();
  }
}

void main() {
  final editor = Editor();
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    try {
      run(editor, line.trim().split(RegExp(r'\s+')));
    } on EditError catch (error) {
      print('error: $error');
    }
  }
}
