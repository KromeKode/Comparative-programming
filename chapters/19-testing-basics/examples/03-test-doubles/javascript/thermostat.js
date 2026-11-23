// Turns the heater on if the room is more than a degree below target, and off if
// it is more than a degree above. Within a degree, it leaves the heater alone.
// Any object with a readCelsius method will do as the sensor, and any with
// turnOn and turnOff methods as the heater.
export function regulate(sensor, heater, target) {
  const celsius = sensor.readCelsius();
  if (celsius < target - 1) {
    heater.turnOn();
  } else if (celsius > target + 1) {
    heater.turnOff();
  }
}
