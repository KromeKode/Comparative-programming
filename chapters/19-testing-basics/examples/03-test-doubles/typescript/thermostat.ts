export interface Sensor {
  readCelsius(): number;
}

export interface Heater {
  turnOn(): void;
  turnOff(): void;
}

// Turns the heater on if the room is more than a degree below target, and off if
// it is more than a degree above. Within a degree, it leaves the heater alone.
export function regulate(sensor: Sensor, heater: Heater, target: number): void {
  const celsius = sensor.readCelsius();
  if (celsius < target - 1) {
    heater.turnOn();
  } else if (celsius > target + 1) {
    heater.turnOff();
  }
}
