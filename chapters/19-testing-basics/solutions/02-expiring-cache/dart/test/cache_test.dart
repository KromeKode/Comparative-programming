import 'package:cache/cache.dart';
import 'package:test/test.dart';

/// A clock that says whatever time the test sets.
class FakeClock implements Clock {
  int time = 0;

  @override
  int now() => time;
}

void main() {
  // setUp runs before each test, so every test gets a new clock.
  late FakeClock clock;
  setUp(() => clock = FakeClock());

  test('put then get', () {
    final cache = Cache(10, clock);
    cache.put('a', '1');
    expect(cache.get('a'), '1');
    expect(cache.get('b'), isNull);
    expect(cache.length, 1);
  });

  // An entry put at time t is there until t + ttl, and gone from then on.
  test('expires after ttl', () {
    clock.time = 100;
    final cache = Cache(10, clock);
    cache.put('a', '1');
    clock.time = 109;
    expect(cache.get('a'), '1');
    clock.time = 110;
    expect(cache.get('a'), isNull);
    expect(cache.length, 0);
  });

  // Getting an entry doesn't make it last longer.
  test('get does not refresh', () {
    final cache = Cache(10, clock);
    cache.put('a', '1');
    clock.time = 5;
    expect(cache.get('a'), '1');
    clock.time = 10;
    expect(cache.get('a'), isNull);
  });

  // Putting a key again replaces its value, and its ttl starts again.
  test('put replaces', () {
    final cache = Cache(10, clock);
    cache.put('a', '1');
    clock.time = 5;
    cache.put('a', '2');
    expect(cache.length, 1);
    clock.time = 14;
    expect(cache.get('a'), '2');
    clock.time = 15;
    expect(cache.get('a'), isNull);
  });

  // Each key has its own time: one expiring, or a put of another, leaves the rest.
  test('keys are separate', () {
    final cache = Cache(10, clock);
    cache.put('a', '1');
    clock.time = 5;
    cache.put('b', '2');
    expect(cache.get('a'), '1');
    expect(cache.get('b'), '2');
    expect(cache.length, 2);
    clock.time = 10;
    expect(cache.get('a'), isNull);
    expect(cache.get('b'), '2');
    expect(cache.length, 1);
  });
}
