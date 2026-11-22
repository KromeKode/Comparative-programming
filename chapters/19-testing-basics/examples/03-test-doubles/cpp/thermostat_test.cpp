#include "check.hpp"
#include "thermostat.hpp"

using thermostat::regulate;

// A stub: a sensor that reads whatever temperature the test gives it.
struct StubSensor : thermostat::Sensor {
    int celsius;
    explicit StubSensor(int celsius) : celsius(celsius) {}
    int read_celsius() override { return celsius; }
};

// A spy: a heater that counts the calls made to it, for the test to check.
struct SpyHeater : thermostat::Heater {
    int ons = 0;
    int offs = 0;
    void turn_on() override { ons++; }
    void turn_off() override { offs++; }
};

TEST(too_cold) {
    StubSensor sensor(17);
    SpyHeater heater;
    regulate(sensor, heater, 20);
    CHECK_EQ(heater.ons, 1);
    CHECK_EQ(heater.offs, 0);
}

TEST(too_hot) {
    StubSensor sensor(23);
    SpyHeater heater;
    regulate(sensor, heater, 20);
    CHECK_EQ(heater.ons, 0);
    CHECK_EQ(heater.offs, 1);
}

TEST(close_enough) {
    for (int celsius = 19; celsius <= 21; celsius++) {
        StubSensor sensor(celsius);
        SpyHeater heater;
        regulate(sensor, heater, 20);
        CHECK_EQ(heater.ons + heater.offs, 0);
    }
}

int main() { return check::run_all_tests(); }
