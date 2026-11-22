// Write tests for the cache here, with test(). One is here to start with, on the
// computer's clock, which can't show what happens as time passes without the test
// waiting for it. The README lists what the cache promises.
import 'package:cache/cache.dart';
import 'package:test/test.dart';

void main() {
  test('put then get', () {
    final cache = Cache(60, const SystemClock());
    cache.put('a', '1');
    expect(cache.get('a'), '1');
  });
}
