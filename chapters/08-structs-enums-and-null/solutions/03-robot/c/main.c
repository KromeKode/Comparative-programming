#include <stdio.h>
#include <string.h>

// Clockwise order, so turning right adds 1 and turning left subtracts 1.
typedef enum { NORTH, EAST, SOUTH, WEST } Direction;
enum { DIRECTION_COUNT = WEST + 1 };

static const char *const NAMES[DIRECTION_COUNT] = {"north", "east", "south", "west"};
static const int DX[DIRECTION_COUNT] = {0, 1, 0, -1};
static const int DY[DIRECTION_COUNT] = {1, 0, -1, 0};

typedef struct {
    long x;
    long y;
    Direction facing;
} Robot;

static Direction turn(Direction direction, int steps) {
    // Adding DIRECTION_COUNT first keeps the left operand of % from going negative.
    return (Direction)((direction + steps + DIRECTION_COUNT) % DIRECTION_COUNT);
}

static void run(Robot *robot, char command) {
    switch (command) {
    case 'L':
        robot->facing = turn(robot->facing, -1);
        break;
    case 'R':
        robot->facing = turn(robot->facing, 1);
        break;
    case 'F':
        robot->x += DX[robot->facing];
        robot->y += DY[robot->facing];
        break;
    }
}

int main(void) {
    static char line[1 << 16];
    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        size_t good = strspn(line, "LRF"); // how many characters at the start are valid
        if (line[good] != '\0') {
            printf("error: unknown command '%c' at position %zu\n", line[good], good + 1);
            continue;
        }
        Robot robot = {0, 0, NORTH};
        for (const char *c = line; *c != '\0'; c++) {
            run(&robot, *c);
        }
        printf("%ld %ld %s\n", robot.x, robot.y, NAMES[robot.facing]);
    }
    return 0;
}
