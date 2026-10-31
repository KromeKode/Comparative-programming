#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// An enum gives names to integers: RED is 0, GREEN is 1, and YELLOW is 2.
typedef enum { RED, GREEN, YELLOW } Light;
enum { LIGHT_COUNT = YELLOW + 1 };

// Anything else a light needs lives in arrays indexed by it.
static const char *const NAMES[LIGHT_COUNT] = {"red", "green", "yellow"};
static const int SECONDS[LIGHT_COUNT] = {30, 25, 5};

static Light next(Light light) {
    return (Light)((light + 1) % LIGHT_COUNT);
}

static const char *meaning(Light light) {
    switch (light) {
    case RED:
        return "stop";
    case GREEN:
        return "go";
    case YELLOW:
        return "slow down";
    }
    return "?"; // a Light variable can hold any int, so C needs a fallback
}

// Sets *out and returns true if text names a light.
static bool parse_light(const char *text, Light *out) {
    for (int i = 0; i < LIGHT_COUNT; i++) {
        if (strcmp(NAMES[i], text) == 0) {
            *out = (Light)i;
            return true;
        }
    }
    return false;
}

static void show_parse(const char *text) {
    Light light;
    if (parse_light(text, &light)) {
        printf("parse \"%s\": %s\n", text, NAMES[light]);
    } else {
        printf("parse \"%s\": not a light\n", text);
    }
}

int main(void) {
    printf("lights:");
    for (int i = 0; i < LIGHT_COUNT; i++) {
        printf(" %s", NAMES[i]);
    }
    printf("\n");
    printf("yellow is number %d\n", YELLOW);

    Light light = RED;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        printf("after %s: %s\n", NAMES[light], NAMES[next(light)]);
        light = next(light);
    }
    printf("red means %s, green means %s, yellow means %s\n", meaning(RED),
           meaning(GREEN), meaning(YELLOW));

    int total = 0;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        total += SECONDS[i];
    }
    printf("one cycle: %d seconds\n", total);

    show_parse("green");
    show_parse("blue");
    return 0;
}
