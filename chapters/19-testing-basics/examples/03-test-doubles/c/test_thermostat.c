#include "check.h"
#include "thermostat.h"

// A stub: a sensor that reads whatever temperature the test gives it.
static int read_stub(void *context) { return *(int *)context; }

static Sensor stub_sensor(int *celsius) { return (Sensor){celsius, read_stub}; }

// A spy: a heater that counts the calls made to it, for the test to check.
typedef struct {
    int ons;
    int offs;
} Calls;

static void count_on(void *context) { ((Calls *)context)->ons++; }
static void count_off(void *context) { ((Calls *)context)->offs++; }

static Heater spy_heater(Calls *calls) { return (Heater){calls, count_on, count_off}; }

static void test_too_cold(void) {
    int celsius = 17;
    Calls calls = {0, 0};
    regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
    CHECK_INT_EQ(calls.ons, 1);
    CHECK_INT_EQ(calls.offs, 0);
}

static void test_too_hot(void) {
    int celsius = 23;
    Calls calls = {0, 0};
    regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
    CHECK_INT_EQ(calls.ons, 0);
    CHECK_INT_EQ(calls.offs, 1);
}

static void test_close_enough(void) {
    for (int celsius = 19; celsius <= 21; celsius++) {
        Calls calls = {0, 0};
        regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
        CHECK_INT_EQ(calls.ons + calls.offs, 0);
    }
}

int main(void) {
    RUN_TEST(test_too_cold);
    RUN_TEST(test_too_hot);
    RUN_TEST(test_close_enough);
    return check_exit_code();
}
