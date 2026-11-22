// Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
//
// Names that start with _ are private to this library, which is this file.
// main.dart can use only Version, parseVersion, compare, Range, and parseRange.

const _number = r'0|[1-9][0-9]{0,14}';
const _identifier = r'(?:0|[1-9][0-9]*|[0-9]*[A-Za-z-][0-9A-Za-z-]*)';
const _pre = '$_identifier(?:\\.$_identifier)*';
final _version = RegExp('^($_number)\\.($_number)\\.($_number)(?:-($_pre))?\$');
const _part = '(?:$_number|[xX*])';
// A partial version, 1 or 1.2 or 1.2.3, with x, X, or * for any part. A pre-release
// can follow only three numbers, which _expand checks.
final _partial = RegExp(
  '^($_part)(?:\\.($_part)(?:\\.($_part)(?:-($_pre))?)?)?\$',
);
final _operator = RegExp(r'^(<=|>=|<|>|=|\^|~)?');
bool _isWild(String part) => part == 'x' || part == 'X' || part == '*';

class Version {
  final int major, minor, patch;

  /// The pre-release's identifiers, if it has one.
  final List<String> pre;

  const Version(this.major, this.minor, this.patch, [this.pre = const []]);

  bool _sameCore(Version other) =>
      major == other.major && minor == other.minor && patch == other.patch;

  @override
  String toString() =>
      '$major.$minor.$patch${pre.isEmpty ? '' : '-${pre.join('.')}'}';
}

/// The version that [text] is, or null if it isn't one.
Version? parseVersion(String text) {
  final match = _version.firstMatch(text);
  if (match == null) return null;
  final [major, minor, patch] = [
    for (var i = 1; i <= 3; i++) int.parse(match[i]!),
  ];
  return Version(major, minor, patch, match[4]?.split('.') ?? const []);
}

final _digits = RegExp(r'^[0-9]+$');

/// Numbers compare as numbers, and come before other identifiers, which compare
/// character by character. Numbers have no 0 in front, so a longer one is larger.
int _compareIdentifiers(String a, String b) {
  final (aNumber, bNumber) = (_digits.hasMatch(a), _digits.hasMatch(b));
  if (aNumber && bNumber) {
    final byLength = a.length.compareTo(b.length);
    return byLength != 0 ? byLength : a.compareTo(b);
  }
  if (aNumber != bNumber) return aNumber ? -1 : 1;
  return a.compareTo(b);
}

/// Negative if [a] comes before [b], 0 if they are equal, and positive if after.
int compare(Version a, Version b) {
  for (final (x, y) in [
    (a.major, b.major),
    (a.minor, b.minor),
    (a.patch, b.patch),
  ]) {
    if (x != y) return x.compareTo(y);
  }
  // A version without a pre-release comes after the same version with one.
  if (a.pre.isEmpty || b.pre.isEmpty) {
    return b.pre.length.compareTo(a.pre.length);
  }
  for (var i = 0; i < a.pre.length && i < b.pre.length; i++) {
    final order = _compareIdentifiers(a.pre[i], b.pre[i]);
    if (order != 0) return order;
  }
  return a.pre.length.compareTo(b.pre.length);
}

enum _Op { eq, lt, le, gt, ge }

/// A simple comparator: an operator and a full version.
typedef _Simple = (_Op, Version);

/// The simple comparators that a comparator stands for, or null if it isn't one.
List<_Simple>? _expand(String token) {
  final op = _operator.firstMatch(token)![0]!;
  final match = _partial.firstMatch(token.substring(op.length));
  if (match == null) return null;
  final parts = [match[1], match[2], match[3]];
  // known counts the numbers, which must come before any wildcards.
  var known = 0;
  while (known < 3 && parts[known] != null && !_isWild(parts[known]!)) {
    known++;
  }
  if (parts.skip(known).any((part) => part != null && !_isWild(part))) {
    return null;
  }
  // A pre-release can follow only three numbers.
  if (match[4] != null && known < 3) return null;
  final nums = [
    for (var i = 0; i < 3; i++) i < known ? int.parse(parts[i]!) : 0,
  ];
  final low = Version(nums[0], nums[1], nums[2], match[4]?.split('.') ?? []);
  // The first version after all those whose parts up to level match: the -0
  // pre-release of the next major, minor, or patch.
  Version after(int level) => switch (level) {
    0 => Version(nums[0] + 1, 0, 0, ['0']),
    1 => Version(nums[0], nums[1] + 1, 0, ['0']),
    _ => Version(nums[0], nums[1], nums[2] + 1, ['0']),
  };
  if (known == 0) {
    // Any version: nothing is outside it, and nothing is above or below it.
    return op == '<' || op == '>'
        ? [
            (_Op.lt, const Version(0, 0, 0, ['0'])),
          ]
        : [];
  }
  final full = known == 3;
  final next = after(known - 1);
  switch (op) {
    case '' || '=':
      return full ? [(_Op.eq, low)] : [(_Op.ge, low), (_Op.lt, next)];
    case '>':
      return full
          ? [(_Op.gt, low)]
          : [(_Op.ge, Version(next.major, next.minor, next.patch))];
    case '>=':
      return [(_Op.ge, low)];
    case '<':
      return [
        (_Op.lt, full ? low : Version(nums[0], nums[1], nums[2], ['0'])),
      ];
    case '<=':
      return full ? [(_Op.le, low)] : [(_Op.lt, next)];
    case '~':
      return [(_Op.ge, low), (_Op.lt, after(known == 1 ? 0 : 1))];
  }
  // ^ allows changes that don't change the first part that isn't 0.
  final first = nums.take(known).toList().indexWhere((num) => num != 0);
  return [(_Op.ge, low), (_Op.lt, after(first >= 0 ? first : known - 1))];
}

bool _holds(Version version, _Simple simple) {
  final (op, bound) = simple;
  final order = compare(version, bound);
  return switch (op) {
    _Op.eq => order == 0,
    _Op.lt => order < 0,
    _Op.le => order <= 0,
    _Op.gt => order > 0,
    _Op.ge => order >= 0,
  };
}

/// Sets of simple comparators joined by ||. A version is in the range if it passes
/// every comparator of some set.
class Range {
  final List<List<_Simple>> _sets;

  Range._(this._sets);

  bool allows(Version version) => _sets.any(
    (simples) =>
        simples.every((simple) => _holds(version, simple)) &&
        // A pre-release is in only if a comparator of the set names a
        // pre-release of the same major, minor, and patch.
        (version.pre.isEmpty ||
            simples.any(
              (simple) =>
                  simple.$2.pre.isNotEmpty && simple.$2._sameCore(version),
            )),
  );

  /// The highest of [versions] that the range allows, or null if it allows none.
  Version? highest(List<Version> versions) {
    Version? best;
    for (final version in versions) {
      if (allows(version) && (best == null || compare(version, best) > 0)) {
        best = version;
      }
    }
    return best;
  }
}

/// The range that [text] is. Throws a [FormatException], saying why, if it isn't
/// one.
Range parseRange(String text) {
  final sets = <List<_Simple>>[];
  for (final side in text.split('||')) {
    final tokens = side.split(RegExp(r'\s+')).where((t) => t.isNotEmpty);
    if (tokens.isEmpty) {
      throw const FormatException('each side of || needs a comparator');
    }
    final simples = <_Simple>[];
    for (final token in tokens) {
      final expanded = _expand(token);
      if (expanded == null) throw FormatException("$token isn't a comparator");
      simples.addAll(expanded);
    }
    sets.add(simples);
  }
  // Range._ is a private constructor, so this library is the only place that can
  // make a Range.
  return Range._(sets);
}
