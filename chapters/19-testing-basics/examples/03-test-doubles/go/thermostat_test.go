package thermostat

import (
	"slices"
	"testing"
)

// A stub: a sensor that reads whatever temperature the test gives it.
type stubSensor int

func (s stubSensor) ReadCelsius() int { return int(s) }

// A spy: a heater that records the calls made to it, for the test to check.
type spyHeater struct {
	calls []string
}

func (h *spyHeater) TurnOn()  { h.calls = append(h.calls, "on") }
func (h *spyHeater) TurnOff() { h.calls = append(h.calls, "off") }

func TestTooCold(t *testing.T) {
	heater := &spyHeater{}
	Regulate(stubSensor(17), heater, 20)
	if !slices.Equal(heater.calls, []string{"on"}) {
		t.Errorf("calls = %q, want [on]", heater.calls)
	}
}

func TestTooHot(t *testing.T) {
	heater := &spyHeater{}
	Regulate(stubSensor(23), heater, 20)
	if !slices.Equal(heater.calls, []string{"off"}) {
		t.Errorf("calls = %q, want [off]", heater.calls)
	}
}

func TestCloseEnough(t *testing.T) {
	for celsius := 19; celsius <= 21; celsius++ {
		heater := &spyHeater{}
		Regulate(stubSensor(celsius), heater, 20)
		if len(heater.calls) != 0 {
			t.Errorf("at %d: calls = %q, want none", celsius, heater.calls)
		}
	}
}
