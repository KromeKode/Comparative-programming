// A mixin is a bundle of members that a class can add with "with". Describe
// declares name without a body, so a class that uses it must supply one.
mixin Describe {
  String get name;

  String describe() => 'this is the $name';
}

// "on Describe" says Logged can only be mixed into a class that already has
// Describe, so Logged can use name. A mixin can also have fields.
mixin Logged on Describe {
  final List<String> _history = [];

  List<String> get history => List.unmodifiable(_history);

  void log(String event) {
    _history.add('$name: $event');
  }
}

class Door with Describe, Logged {
  @override
  final String name;

  Door(this.name);

  void open() => log('opened');

  void close() => log('closed');
}

class Light with Describe, Logged {
  @override
  final String name;
  bool on = false;

  Light(this.name);

  void switchOn() {
    on = true;
    log('switched on');
  }

  // Light replaces the default describe, and reuses it.
  @override
  String describe() => '${super.describe()}, and it is ${on ? 'on' : 'off'}';
}

void main() {
  final door = Door('front door');
  final light = Light('porch light');
  door.open();
  door.close();
  light.switchOn();

  for (final Logged thing in [door, light]) {
    print(thing.describe());
    for (final line in thing.history) {
      print('  $line');
    }
  }
}
