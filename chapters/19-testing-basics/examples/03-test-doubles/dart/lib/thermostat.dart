abstract interface class Sensor {
  int readCelsius();
}

abstract interface class Heater {
  void turnOn();
  void turnOff();
}

/// Turns the heater on if the room is more than a degree below [target], and off
/// if it is more than a degree above. Within a degree, it leaves the heater alone.
void regulate(Sensor sensor, Heater heater, int target) {
  final celsius = sensor.readCelsius();
  if (celsius < target - 1) {
    heater.turnOn();
  } else if (celsius > target + 1) {
    heater.turnOff();
  }
}
