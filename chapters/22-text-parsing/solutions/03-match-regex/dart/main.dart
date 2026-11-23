// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
import 'dart:convert';
import 'dart:io';

/// What a "\" can make a literal: the characters that mean something, inside brackets
/// or out.
const special = r'\.[]()*+?|^$-';

class PatternError implements Exception {
  final int at; // an index into the pattern's characters
  final String message;
  PatternError(this.at, this.message);
}

/// The tree. A character, a ".", and brackets are each a set of characters: "a" is the
/// range a to a, and "." is the empty set, negated.
sealed class Node {}

class Chars extends Node {
  final List<(int, int)> ranges; // of code points
  final bool negated;
  Chars(this.ranges, {this.negated = false});
}

class Anchor extends Node {
  final String kind; // "^" or "$"
  Anchor(this.kind);
}

class Concat extends Node {
  final List<Node> items;
  Concat(this.items);
}

class Alternation extends Node {
  final List<Node> options;
  Alternation(this.options);
}

class Repeat extends Node {
  final Node item;
  final String kind; // "*", "+", or "?"
  Repeat(this.item, this.kind);
}

/// Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?].
class Parser {
  // A String is UTF-16, so these are its code points, one to a string, and an index
  // counts characters, as columns do.
  final List<String> pattern;
  int i = 0;

  Parser(String pattern)
    : pattern = pattern.runes.map(String.fromCharCode).toList();

  String? peek() => i < pattern.length ? pattern[i] : null;

  Node parse() {
    final node = alternation();
    if (i < pattern.length) {
      // alternation() stops early only at a ")".
      throw PatternError(i, 'unmatched ")"');
    }
    return node;
  }

  Node alternation() {
    final options = [concat()];
    while (peek() == '|') {
      i++;
      options.add(concat());
    }
    return options.length == 1 ? options.single : Alternation(options);
  }

  Node concat() {
    final items = <Node>[];
    for (var c = peek(); c != null && c != '|' && c != ')'; c = peek()) {
      items.add(repeat());
    }
    return Concat(items);
  }

  Node repeat() {
    // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
    if (peek() case '*' || '+' || '?') {
      throw PatternError(i, 'nothing to repeat');
    }
    if (peek() case final anchor? when anchor == '^' || anchor == r'$') {
      i++;
      return Anchor(anchor);
    }
    final item = atom();
    if (peek() case final kind? when '*+?'.contains(kind)) {
      i++;
      return Repeat(item, kind);
    }
    return item;
  }

  Node atom() {
    switch (peek()) {
      case '(':
        final start = i;
        i++;
        final node = alternation();
        if (peek() != ')') throw PatternError(start, 'unclosed "("');
        i++;
        return node;
      case '[':
        return brackets();
      case '.':
        i++;
        return Chars([], negated: true);
    }
    final c = char();
    return Chars([(c, c)]);
  }

  /// A character, or a "\" and the character it makes a literal, as a code point.
  int char() {
    final c = pattern[i];
    if (c != r'\') {
      i++;
      return c.runes.single;
    }
    if (i + 1 == pattern.length) throw PatternError(i, 'trailing backslash');
    final escaped = pattern[i + 1];
    if (!special.contains(escaped)) throw PatternError(i, 'invalid escape');
    i += 2;
    return escaped.runes.single;
  }

  Node brackets() {
    final start = i;
    i++; // the [
    final negated = peek() == '^';
    if (negated) i++;
    if (peek() == ']') throw PatternError(start, 'empty brackets');
    final ranges = <(int, int)>[];
    while (peek() != ']') {
      if (peek() == null) throw PatternError(start, 'unclosed "["');
      final at = i;
      final low = char();
      var high = low;
      // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
      if (peek() == '-' && i + 1 < pattern.length && pattern[i + 1] != ']') {
        i++;
        high = char();
        if (high < low) throw PatternError(at, 'range out of order');
      }
      ranges.add((low, high));
    }
    i++; // the ]
    return Chars(ranges, negated: negated);
  }
}

/// The program. Op.chars reads a character, if it's in the set, and goes on to the next
/// instruction; Op.split goes on to both x and y; Op.jump goes on to x; Op.start and
/// Op.end go on if the position is the start or the end of the line; and Op.match is a
/// match.
enum Op { chars, split, jump, start, end, match }

class Instruction {
  final Op op;
  final List<(int, int)> ranges;
  final bool negated;
  int x, y;

  Instruction(
    this.op, {
    this.ranges = const [],
    this.negated = false,
    this.x = 0,
    this.y = 0,
  });

  bool accepts(int c) =>
      ranges.any((range) => range.$1 <= c && c <= range.$2) != negated;
}

void compile(Node node, List<Instruction> program) {
  final start = program.length;
  switch (node) {
    case Chars(:final ranges, :final negated):
      program.add(Instruction(Op.chars, ranges: ranges, negated: negated));
    case Anchor(:final kind):
      program.add(Instruction(kind == '^' ? Op.start : Op.end));
    case Concat(:final items):
      for (final item in items) {
        compile(item, program);
      }
    case Alternation(:final options):
      // split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
      final jumps = <Instruction>[];
      for (final option in options.sublist(0, options.length - 1)) {
        final split = Instruction(Op.split, x: program.length + 1);
        program.add(split);
        compile(option, program);
        jumps.add(Instruction(Op.jump));
        program.add(jumps.last);
        split.y = program.length;
      }
      compile(options.last, program);
      for (final jump in jumps) {
        jump.x = program.length;
      }
    case Repeat(:final item, kind: '*'):
      // L1: split L2, L3; L2: item; jump L1; L3:
      final split = Instruction(Op.split, x: start + 1);
      program.add(split);
      compile(item, program);
      program.add(Instruction(Op.jump, x: start));
      split.y = program.length;
    case Repeat(:final item, kind: '+'):
      // L1: item; split L1, L2; L2:
      compile(item, program);
      program.add(Instruction(Op.split, x: start, y: program.length + 1));
    case Repeat(:final item):
      // split L1, L2; L1: item; L2:
      final split = Instruction(Op.split, x: start + 1);
      program.add(split);
      compile(item, program);
      split.y = program.length;
  }
}

List<Instruction> compilePattern(String pattern) {
  final program = <Instruction>[];
  compile(Parser(pattern).parse(), program);
  program.add(Instruction(Op.match));
  return program;
}

/// The Op.chars instructions that pcs lead to without reading a character, at the
/// position at, and whether one of them leads to Op.match.
(List<int>, bool) follow(
  List<Instruction> program,
  List<int> pcs,
  int at,
  int length,
) {
  final waiting = <int>[];
  final seen = <int>{};
  final stack = [...pcs];
  while (stack.isNotEmpty) {
    final pc = stack.removeLast();
    // add is false if the set had it already, so that a loop such as (a*)* doesn't go
    // around forever.
    if (!seen.add(pc)) continue;
    final instruction = program[pc];
    switch (instruction.op) {
      case Op.chars:
        waiting.add(pc);
      case Op.split:
        stack.addAll([instruction.y, instruction.x]);
      case Op.jump:
        stack.add(instruction.x);
      case Op.start:
        if (at == 0) stack.add(pc + 1);
      case Op.end:
        if (at == length) stack.add(pc + 1);
      case Op.match:
        return (const [], true);
    }
  }
  return (waiting, false);
}

/// Whether a match starts anywhere in the line, of code points: instruction 0 starts at
/// each position.
bool search(List<Instruction> program, List<int> line) {
  var waiting = <int>[];
  for (var at = 0; at <= line.length; at++) {
    final (reached, matched) = follow(
      program,
      [...waiting, 0],
      at,
      line.length,
    );
    if (matched) return true;
    waiting = [
      if (at < line.length)
        for (final pc in reached)
          if (program[pc].accepts(line[at])) pc + 1,
    ];
  }
  return false;
}

Future<void> main(List<String> args) async {
  if (args.length != 1) {
    stderr.writeln('usage: rematch PATTERN');
    exit(2);
  }
  final List<Instruction> program;
  try {
    program = compilePattern(args[0]);
  } on PatternError catch (err) {
    stderr.writeln('rematch: column ${err.at + 1}: ${err.message}');
    exit(2);
  }
  final lines = (await utf8.decodeStream(stdin)).split('\n');
  if (lines.last == '') {
    lines.removeLast(); // what comes after the last "\n", or the empty input
  }
  final found = [
    for (final line in lines)
      if (search(program, line.runes.toList())) line,
  ];
  stdout.write(found.map((line) => '$line\n').join());
  exitCode = found.isEmpty ? 1 : 0;
}
