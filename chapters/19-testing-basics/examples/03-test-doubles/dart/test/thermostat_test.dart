import 'package:test/test.dart';
import 'package:thermostat/thermostat.dart';

/// A stub: a sensor that reads whatever temperature the test gives it.
class StubSensor implements Sensor {
  final int celsius;
  StubSensor(this.celsius);

  @override
  int readCelsius() => celsius;
}

/// A spy: a heater that records the calls made to it, for the test to check.
class SpyHeater implements Heater {
  final calls = <String>[];

  @override
  void turnOn() => calls.add('on');

  @override
  void turnOff() => calls.add('off');
}

void main() {
  test('too cold', () {
    final heater = SpyHeater();
    regulate(StubSensor(17), heater, 20);
    expect(heater.calls, ['on']);
  });

  test('too hot', () {
    final heater = SpyHeater();
    regulate(StubSensor(23), heater, 20);
    expect(heater.calls, ['off']);
  });

  test('close enough', () {
    for (final celsius in [19, 20, 21]) {
      final heater = SpyHeater();
      regulate(StubSensor(celsius), heater, 20);
      expect(heater.calls, isEmpty, reason: 'at $celsius');
    }
  });
}
