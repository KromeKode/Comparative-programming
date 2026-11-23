#pragma once

namespace thermostat {

// Interfaces are classes whose functions are all pure virtual.
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual int read_celsius() = 0;
};

class Heater {
public:
    virtual ~Heater() = default;
    virtual void turn_on() = 0;
    virtual void turn_off() = 0;
};

// Turns the heater on if the room is more than a degree below target, and off if
// it is more than a degree above. Within a degree, it leaves the heater alone.
void regulate(Sensor& sensor, Heater& heater, int target);

}  // namespace thermostat
