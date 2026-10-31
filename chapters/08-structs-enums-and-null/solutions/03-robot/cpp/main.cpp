#include <array>
#include <iostream>
#include <string>
#include <string_view>

// Clockwise order, so turning right adds 1 and turning left subtracts 1.
enum class Direction { North, East, South, West };

constexpr int DIRECTION_COUNT = 4;
constexpr std::array<std::string_view, DIRECTION_COUNT> NAMES = {"north", "east", "south",
                                                                 "west"};
constexpr std::array<int, DIRECTION_COUNT> DX = {0, 1, 0, -1};
constexpr std::array<int, DIRECTION_COUNT> DY = {1, 0, -1, 0};

Direction turn(Direction direction, int steps) {
    // Adding DIRECTION_COUNT first keeps the left operand of % from going negative.
    int number = static_cast<int>(direction) + steps + DIRECTION_COUNT;
    return static_cast<Direction>(number % DIRECTION_COUNT);
}

struct Robot {
    long x = 0;
    long y = 0;
    Direction facing = Direction::North;

    void run(char command) {
        auto index = static_cast<std::size_t>(facing);
        switch (command) {
        case 'L':
            facing = turn(facing, -1);
            break;
        case 'R':
            facing = turn(facing, 1);
            break;
        case 'F':
            x += DX[index];
            y += DY[index];
            break;
        }
    }
};

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        auto bad = line.find_first_not_of("LRF");
        if (bad != std::string::npos) {
            std::cout << "error: unknown command '" << line[bad] << "' at position " << bad + 1
                      << '\n';
            continue;
        }
        Robot robot; // every field starts at its default from the struct
        for (char command : line) {
            robot.run(command);
        }
        std::cout << robot.x << ' ' << robot.y << ' '
                  << NAMES[static_cast<std::size_t>(robot.facing)] << '\n';
    }
}
