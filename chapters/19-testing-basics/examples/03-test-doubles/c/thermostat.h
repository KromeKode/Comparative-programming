#ifndef THERMOSTAT_H
#define THERMOSTAT_H

// C's interfaces are structs of function pointers. Each function gets the
// context pointer, which points at whatever the implementation needs.
typedef struct {
    void *context;
    int (*read_celsius)(void *context);
} Sensor;

typedef struct {
    void *context;
    void (*turn_on)(void *context);
    void (*turn_off)(void *context);
} Heater;

// Turns the heater on if the room is more than a degree below target, and off if
// it is more than a degree above. Within a degree, it leaves the heater alone.
void regulate(Sensor sensor, Heater heater, int target);

#endif
