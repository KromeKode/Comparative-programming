#include <iostream>
#include <string>
#include <vector>

struct Player {
    std::string name;
    std::vector<int> scores;
};

std::string show(const Player &p) {
    std::string out = p.name;
    for (int score : p.scores) {
        out += " " + std::to_string(score);
    }
    return out;
}

// A raw pointer member is copied as an address, just as in C.
struct Counter {
    int *count;
};

int main() {
    // Copying a struct copies each member, and std::string and std::vector
    // copy their own heap memory, so the copy is deep.
    Player a{"Ada", {90, 85}};
    Player b = a;
    b.name[0] = 'E';
    b.scores.push_back(70);
    std::cout << "b = a copies everything: a = " << show(a) << ", b = " << show(b)
              << "\n";

    int shared = 1;
    Counter c{&shared};
    Counter d = c;
    *d.count = 2;
    std::cout << "a copied pointer still points at the same int: *c.count = "
              << *c.count << "\n";

    std::vector<Player> team = {a, b};
    for (auto p : team) { // p is a copy of each player
        p.name += "?";
    }
    std::cout << "for (auto p : team) changes copies: " << team[0].name << " "
              << team[1].name << "\n";
    for (auto &p : team) { // p refers to each player
        p.name += "!";
    }
    std::cout << "for (auto &p : team) changes the players: " << team[0].name << " "
              << team[1].name << "\n";
}
