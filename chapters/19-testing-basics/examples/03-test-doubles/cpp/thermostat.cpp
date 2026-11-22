#include "thermostat.hpp"

namespace thermostat {

void regulate(Sensor& sensor, Heater& heater, int target) {
    int celsius = sensor.read_celsius();
    if (celsius < target - 1) {
        heater.turn_on();
    } else if (celsius > target + 1) {
        heater.turn_off();
    }
}

}  // namespace thermostat
