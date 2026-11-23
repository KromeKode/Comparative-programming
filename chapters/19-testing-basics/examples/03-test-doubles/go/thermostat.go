// Package thermostat keeps a room near a target temperature.
package thermostat

// A Sensor reads the temperature.
type Sensor interface {
	ReadCelsius() int
}

// A Heater can be turned on and off.
type Heater interface {
	TurnOn()
	TurnOff()
}

// Regulate turns the heater on if the room is more than a degree below target,
// and off if it is more than a degree above. Within a degree, it leaves the
// heater alone.
func Regulate(sensor Sensor, heater Heater, target int) {
	celsius := sensor.ReadCelsius()
	if celsius < target-1 {
		heater.TurnOn()
	} else if celsius > target+1 {
		heater.TurnOff()
	}
}
