import 'dart:io';

class SettingError implements Exception {
  final String message;
  SettingError(this.message);

  @override
  String toString() => message;
}

// One setting whose value is a T. It knows how to read a T from text and how
// to show one. == on two lists asks whether they are the same list, so a
// setting can bring its own equal.
class Setting<T> {
  final String name;
  final T _default;
  T _value;
  final T? Function(String) _parse;
  final String Function(T) _show;
  final String _needs;
  final bool Function(T, T) _equal;

  Setting(
    this.name,
    this._default,
    this._parse,
    this._show,
    this._needs, [
    bool Function(T, T)? equal,
  ]) : _value = _default,
       _equal = equal ?? ((a, b) => a == b);

  T get value => _value;

  void setText(String text) {
    _value =
        _parse(text) ??
        (throw SettingError('$name needs $_needs, not "$text"'));
  }

  void reset() => _value = _default;

  String describe() => '$name = ${_show(_value)}';

  bool isDefault() => _equal(_value, _default);
}

final whole = RegExp(r'^-?[0-9]+$');

int? parseInt(String text) {
  final value = whole.hasMatch(text) ? int.tryParse(text) : null;
  return value != null && value >= -(1 << 31) && value < (1 << 31)
      ? value
      : null;
}

bool? parseBool(String text) => switch (text) {
  'true' => true,
  'false' => false,
  _ => null,
};

bool sameItems(List<String> a, List<String> b) {
  if (a.length != b.length) return false;
  for (var i = 0; i < a.length; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

class Settings {
  // Typed fields: volume.value is an int and muted.value a bool.
  final volume = Setting<int>(
    'volume',
    5,
    parseInt,
    (n) => '$n',
    'a whole number',
  );
  final muted = Setting<bool>(
    'muted',
    false,
    parseBool,
    (b) => '$b',
    'true or false',
  );
  final name = Setting<String>(
    'name',
    'guest',
    (text) => text,
    (text) => text,
    '',
  );
  final tags = Setting<List<String>>(
    'tags',
    [],
    (text) => text.split(' '),
    (tags) => '[${tags.join(', ')}]',
    '',
    sameItems,
  );

  // Dart's generics are covariant, so a Setting<int> is a Setting<Object>,
  // and one list can hold all four.
  late final List<Setting<Object>> all = [volume, muted, name, tags];

  Setting<Object> find(String wanted) {
    for (final setting in all) {
      if (setting.name == wanted) return setting;
    }
    throw SettingError('no setting named "$wanted"');
  }

  String summary() => muted.value
      ? '${name.value} has muted the sound'
      : '${name.value} hears the sound at ${volume.value * 10}%';
}

void run(Settings settings, List<String> words) {
  switch (words) {
    case ['set', final wanted, ...final value]:
      final setting = settings.find(wanted);
      setting.setText(value.join(' '));
      print(setting.describe());
    case ['get', final wanted]:
      print(settings.find(wanted).describe());
    case ['reset', final wanted]:
      final setting = settings.find(wanted);
      setting.reset();
      print(setting.describe());
    case ['list']:
      for (final setting in settings.all) {
        print(setting.describe() + (setting.isDefault() ? ' (default)' : ''));
      }
    case ['summary']:
      print(settings.summary());
  }
}

void main() {
  final settings = Settings();
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    try {
      run(settings, line.trim().split(RegExp(r'\s+')));
    } on SettingError catch (error) {
      print('error: $error');
    }
  }
}
