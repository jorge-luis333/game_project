#pragma once
#include "Player.h"
#include <deque>
#include <string>

// Menu, turn loop and screens. Owns no game rules itself.
class Game {
public:
    void run();

private:
    void play();
    void showRules() const;
    void render(const Player& human, const Player& enemy, int turn) const;
    void showGameOver(const Player& human, const Player& enemy, int turns) const;
    void addLog(const std::string& line);
    static std::string describe(const Player& attacker, bool isHuman, Coord target, const Shot& shot);

    std::deque<std::string> log_;
    Coord lastHumanShot_{-1, -1};
    Coord lastEnemyShot_{-1, -1};
};
