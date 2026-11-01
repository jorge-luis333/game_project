#include "Game.h"
#include "Console.h"
#include <chrono>
#include <memory>
#include <thread>

using namespace Console;

void Game::run() {
    init();
    while (true) {
        clear();
        banner();
        print("      [1] New Game      [2] How to Play      [3] Quit\n", BOLD);
        std::string choice = readCommand("  Select > ");
        if (choice == "1") play();
        else if (choice == "2") showRules();
        else if (choice == "3" || choice == "Q") break;
    }
    print("\n  Fair winds, Captain.\n", CYAN);
}

void Game::play() {
    log_.clear();
    lastHumanShot_ = lastEnemyShot_ = {-1, -1};

    clear();
    title("NEW GAME");
    std::string name = readLine("  Enter your name, Captain > ");
    if (name.empty()) name = "Captain";

    HumanPlayer human(name);
    human.deployFleet();
    std::unique_ptr<Player> enemy = std::make_unique<HunterKillerAI>();   // used through the base class
    addLog(std::string(YELLOW) + "Enemy fleet detected. Commence firing!" + RESET);

    int turn = 1;
    while (true) {
        render(human, *enemy, turn);
        Coord target = human.chooseTarget(enemy->board());
        Shot shot = human.attack(*enemy, target);
        lastHumanShot_ = target;
        addLog(describe(human, true, target, shot));
        if (enemy->hasLost()) break;

        render(human, *enemy, turn);
        print("  " + enemy->name() + " is calculating a firing solution...", GREY);
        std::this_thread::sleep_for(std::chrono::milliseconds(700));

        target = enemy->chooseTarget(human.board());
        shot = enemy->attack(human, target);
        lastEnemyShot_ = target;
        addLog(describe(*enemy, false, target, shot));
        if (human.hasLost()) break;
        ++turn;
    }
    showGameOver(human, *enemy, turn);
}

void Game::render(const Player& human, const Player& enemy, int turn) const {
    clear();
    title("TURN " + std::to_string(turn) + "   |   Captain " + human.name() + "  vs  " + enemy.name());
    draw(human.board(), enemy.board(), false, lastEnemyShot_, lastHumanShot_);
    print("  BATTLE LOG", BOLD);
    for (const std::string& line : log_) print("   " + line);
    print("");
}

void Game::showGameOver(const Player& human, const Player& enemy, int turns) const {
    clear();
    bool won = enemy.hasLost();
    endBanner(won);
    print(won ? "  The enemy fleet rests at the bottom of the sea. Well done, Captain " + human.name() + "!"
              : "  Your fleet has been destroyed. " + enemy.name() + " rules the waves.",
          won ? GREEN : RED);
    print("");
    draw(human.board(), enemy.board(), true, lastEnemyShot_, lastHumanShot_);

    auto row = [](std::string label, const std::string& a, const std::string& b) {
        label.resize(18, ' ');
        std::string left = a;
        left.resize(18, ' ');
        print("  " + label + left + b);
    };
    print("  AFTER-ACTION REPORT  (" + std::to_string(turns) + " turns)", BOLD);
    row("", human.name(), enemy.name());
    row("Shots fired", std::to_string(human.shots()), std::to_string(enemy.shots()));
    row("Hits", std::to_string(human.hits()), std::to_string(enemy.hits()));
    row("Accuracy", std::to_string(human.accuracy()) + "%", std::to_string(enemy.accuracy()) + "%");
    print("");
    pause();
}

void Game::showRules() const {
    clear();
    title("HOW TO PLAY");
    print("  * Each side hides 5 ships on a 10x10 grid:");
    print("      Carrier (5)  Battleship (4)  Cruiser (3)  Submarine (3)  Destroyer (2)");
    print("  * Take turns firing at a coordinate such as B7 (row A-J, column 1-10).");
    print("  * A shot is a HIT or a MISS. Hit every cell of a ship to SINK it.");
    print("  * Sink the whole enemy fleet before HunterKillerAI sinks yours.\n");
    print("  About your opponent:", YELLOW);
    print("  HunterKillerAI sweeps the sea in a checkerboard pattern (hunt mode).");
    print("  Once it scores a hit it closes in on that ship until it sinks (kill mode).\n");
    pause();
}

void Game::addLog(const std::string& line) {
    log_.push_back(line);
    if (log_.size() > 6) log_.pop_front();
}

std::string Game::describe(const Player& attacker, bool isHuman, Coord target, const Shot& shot) {
    std::string msg = isHuman ? std::string(CYAN) + "[YOU]   " + RESET
                              : std::string(MAGENTA) + "[ENEMY] " + RESET + attacker.name() + " fires at ";
    if (isHuman) msg += "You fire at ";
    msg += toString(target) + " ... ";

    switch (shot.result) {
        case ShotResult::Miss:
            return msg + GREY + "splash. Miss." + RESET;
        case ShotResult::Hit:
            return msg + RED + BOLD + (isHuman ? "HIT!" : "HIT on your " + shot.ship->name() + "!") + RESET;
        case ShotResult::Sunk:
            return msg + RED + BOLD + (isHuman ? "You sank their " + shot.ship->name() + "!"
                                               : "Your " + shot.ship->name() + " has been sunk!") + RESET;
        default:
            return msg + "no effect.";
    }
}
