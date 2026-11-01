import 'dart:io';

class PluginError implements Exception {
  final String message;

  PluginError(this.message);

  @override
  String toString() => message;
}

// Every plugin has a name and changes a list of words. describe has a body,
// so it is the default for every plugin that extends Plugin.
abstract class Plugin {
  String get name;

  List<String> apply(List<String> words);

  String describe() => name;
}

class Upper extends Plugin {
  @override
  String get name => 'upper';

  @override
  List<String> apply(List<String> words) => [
    for (final word in words) word.toUpperCase(),
  ];
}

class Lower extends Plugin {
  @override
  String get name => 'lower';

  @override
  List<String> apply(List<String> words) => [
    for (final word in words) word.toLowerCase(),
  ];
}

class Reverse extends Plugin {
  @override
  String get name => 'reverse';

  @override
  List<String> apply(List<String> words) => words.reversed.toList();
}

class Censor extends Plugin {
  final String word;

  Censor(this.word);

  @override
  String get name => 'censor';

  @override
  List<String> apply(List<String> words) => [
    for (final w in words)
      w.toLowerCase() == word.toLowerCase() ? '*' * w.length : w,
  ];

  @override
  String describe() => 'censor($word)';
}

typedef Make = Plugin Function(List<String> args);

class Entry {
  final String usage;
  final String help;
  final Make make;

  Entry(this.usage, this.help, this.make);
}

// Maps a plugin's name to a way to make one. The rest of the program finds
// plugins only through here. A Dart map keeps keys in the order added.
class Registry {
  final _entries = <String, Entry>{};

  void register(String name, Entry entry) {
    _entries[name] = entry;
  }

  Plugin create(String name, List<String> args) {
    final entry = _entries[name];
    if (entry == null) throw PluginError('no plugin named $name');
    return entry.make(args);
  }

  Iterable<Entry> get entries => _entries.values;
}

Make noArguments(Plugin Function() make) => (args) {
  final plugin = make();
  if (args.isNotEmpty) throw PluginError('${plugin.name} takes no arguments');
  return plugin;
};

Plugin makeCensor(List<String> args) {
  if (args.length != 1) throw PluginError('censor needs exactly one word');
  return Censor(args.single);
}

Registry makeRegistry() => Registry()
  ..register(
    'upper',
    Entry('upper', 'make every letter uppercase', noArguments(Upper.new)),
  )
  ..register(
    'lower',
    Entry('lower', 'make every letter lowercase', noArguments(Lower.new)),
  )
  ..register(
    'reverse',
    Entry(
      'reverse',
      'reverse the order of the words',
      noArguments(Reverse.new),
    ),
  )
  ..register(
    'censor',
    Entry('censor WORD', 'hide WORD wherever it appears', makeCensor),
  );

void main() {
  final registry = makeRegistry();
  final pipeline = <Plugin>[];
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    switch (words) {
      case ['list']:
        for (final entry in registry.entries) {
          print('${entry.usage}: ${entry.help}');
        }
      case ['add', final name, ...final args]:
        try {
          final plugin = registry.create(name, args);
          pipeline.add(plugin);
          print('added ${plugin.describe()}');
        } on PluginError catch (error) {
          print('error: $error');
        }
      case ['pipeline']:
        final steps = pipeline.map((plugin) => plugin.describe()).join(' | ');
        print('pipeline: ${steps.isEmpty ? '(empty)' : steps}');
      case ['run', ...final rest]:
        var text = rest;
        for (final plugin in pipeline) {
          text = plugin.apply(text);
        }
        print(text.join(' '));
    }
  }
}
