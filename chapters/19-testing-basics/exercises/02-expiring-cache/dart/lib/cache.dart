/// Something that tells the time, in whole seconds.
abstract interface class Clock {
  int now();
}

/// The computer's clock, which counts the seconds since 1970.
class SystemClock implements Clock {
  const SystemClock();

  @override
  int now() => DateTime.now().millisecondsSinceEpoch ~/ 1000;
}

class _Entry {
  _Entry(this.value, this.expires);

  final String value;
  final int expires; // the time the entry stops being live

  bool isLive(int now) => now < expires;
}

/// Keeps each value for [ttl] seconds after it was put, and then forgets it.
class Cache {
  Cache(this.ttl, this._clock);

  final int ttl;
  final Clock _clock;
  final _entries = <String, _Entry>{};

  /// Stores [value] under [key] for the next [ttl] seconds, replacing any value there.
  void put(String key, String value) {
    final now = _clock.now();
    _entries.removeWhere((_, entry) => !entry.isLive(now));
    _entries[key] = _Entry(value, now + ttl);
  }

  /// The value under [key], or null if there is none or it has expired.
  String? get(String key) {
    final entry = _entries[key];
    if (entry == null || !entry.isLive(_clock.now())) return null;
    return entry.value;
  }

  /// How many entries haven't expired.
  int get length {
    final now = _clock.now();
    return _entries.values.where((entry) => entry.isLive(now)).length;
  }
}
