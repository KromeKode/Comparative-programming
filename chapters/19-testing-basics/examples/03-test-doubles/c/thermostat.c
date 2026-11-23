#include "thermostat.h"

void regulate(Sensor sensor, Heater heater, int target) {
    int celsius = sensor.read_celsius(sensor.context);
    if (celsius < target - 1) {
        heater.turn_on(heater.context);
    } else if (celsius > target + 1) {
        heater.turn_off(heater.context);
    }
}
