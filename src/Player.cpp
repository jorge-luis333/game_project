#include "Player.h"
#include "Console.h"

// ---- Player ----------------------------------------------------------------

Player::Player(std::string name, Board board)
    : name_(std::move(name)), board_(std::move(board)) {}      // Board move constructor

Shot Player::attack(Player& enemy, Coord target) {
    Shot shot = enemy.board_.fire(target);
    ++shots_;
    if (shot.result == ShotResult::Hit || shot.result == ShotResult::Sunk) ++hits_;
    onShotResult(target, shot);
    return shot;
}

// ---- HumanPlayer -----------------------------------------------------------

HumanPlayer::HumanPlayer(std::string name) : Player(std::move(name), Board()) {}

void HumanPlayer::deployFleet() {
    while (true) {
        Console::clear();
        Console::title("FLEET DEPLOYMENT  |  Captain " + name_);
        Console::print("  [A] Auto-deploy fleet      [M] Place ships manually\n");
        std::string choice = Console::readCommand("  Select > ");
        if (choice == "A") return autoDeploy();
        if (choice == "M") return manualDeploy();
    }
}

void HumanPlayer::autoDeploy() {
    while (true) {
        board_ = Board::randomFleet();                         // Board move assignment
        Console::clear();
        Console::title("FLEET DEPLOYMENT  |  Auto");
        Console::draw(board_, "YOUR FLEET");
        if (Console::readCommand("  [Enter] Accept    [R] Re-roll  > ") != "R") return;
    }
}

void HumanPlayer::manualDeploy() {
    const std::vector<Ship> fleet = Board::standardFleet();
    std::vector<Board> history;                                // snapshots for undo
    std::string error;

    while (board_.ships().size() < fleet.size()) {
        const Ship& next = fleet[board_.ships().size()];
        Console::clear();
        Console::title("FLEET DEPLOYMENT  |  Manual");
        Console::draw(board_, "YOUR FLEET");
        Console::print("  Placing: " + next.name() + " (" + std::to_string(next.size()) + " cells)", Console::YELLOW);
        Console::print("  Type start + direction, e.g. 'A1 H' (horizontal) or 'C5 V' (vertical).  [U] Undo");
        if (!error.empty()) Console::print("  " + error, Console::RED);
        error.clear();

        std::string in = Console::readCommand("  > ");
        if (in == "U") {
            if (history.empty()) { error = "Nothing to undo."; continue; }
            board_ = history.back();                           // Board copy assignment
            history.pop_back();
            continue;
        }

        Coord start;
        if (in.size() < 3 || (in.back() != 'H' && in.back() != 'V') ||
            !Console::parseCoord(in.substr(0, in.size() - 1), start)) {
            error = "Invalid input. Example: B3 V";
            continue;
        }

        history.push_back(board_);                             // Board copy constructor
        if (!board_.placeShip(next, start, in.back() == 'H')) {
            history.pop_back();
            error = "The " + next.name() + " does not fit there.";
        }
    }

    Console::clear();
    Console::title("FLEET DEPLOYMENT  |  Complete");
    Console::draw(board_, "YOUR FLEET");
    Console::pause();
}

Coord HumanPlayer::chooseTarget(const Board& enemy) {
    while (true) {
        Coord c;
        std::string in = Console::readCommand("  Target coordinates (e.g. B7) > ");
        if (!Console::parseCoord(in, c))
            Console::print("  Invalid target. Use a letter A-J and a number 1-10.", Console::RED);
        else if (enemy.wasShot(c))
            Console::print("  You already fired at " + Console::toString(c) + ".", Console::YELLOW);
        else
            return c;
    }
}

// ---- ComputerPlayer --------------------------------------------------------

ComputerPlayer::ComputerPlayer(std::string name) : Player(std::move(name), Board::randomFleet()) {}

Coord ComputerPlayer::chooseTarget(const Board& enemy) {
    return randomUntried(enemy, false);
}

// Only looks at hit/miss marks, never at hidden ships: the AI does not cheat.
Coord ComputerPlayer::randomUntried(const Board& enemy, bool checkerboard) const {
    std::vector<Coord> options;
    for (int r = 0; r < Board::SIZE; ++r)
        for (int c = 0; c < Board::SIZE; ++c)
            if (!enemy.wasShot({r, c}) && (!checkerboard || (r + c) % 2 == 0))
                options.push_back({r, c});

    if (options.empty()) return checkerboard ? randomUntried(enemy, false) : Coord{0, 0};
    return options[randomInt(0, static_cast<int>(options.size()) - 1)];
}

// ---- HunterKillerAI --------------------------------------------------------

HunterKillerAI::HunterKillerAI() : ComputerPlayer("HunterKillerAI") {}

Coord HunterKillerAI::chooseTarget(const Board& enemy) {
    while (!targets_.empty()) {                                // kill mode
        Coord c = targets_.front();
        targets_.pop_front();
        if (Board::inBounds(c) && !enemy.wasShot(c)) return c;
    }
    return randomUntried(enemy, true);                         // hunt mode
}

void HunterKillerAI::onShotResult(Coord target, const Shot& shot) {
    if (shot.result == ShotResult::Hit) {
        openHits_.push_back(target);
    } else if (shot.result == ShotResult::Sunk) {
        for (Coord c : shot.ship->cells())
            openHits_.erase(std::remove(openHits_.begin(), openHits_.end(), c), openHits_.end());
    } else {
        return;                                                // a miss keeps the current plan
    }
    rebuildTargets();
}

void HunterKillerAI::rebuildTargets() {
    targets_.clear();
    if (openHits_.empty()) return;                             // back to hunting

    // Two or more hits in a line: extend both ends of the line first.
    if (openHits_.size() >= 2) {
        const Coord first = openHits_[0];
        bool sameRow = std::all_of(openHits_.begin(), openHits_.end(), [&](Coord c) { return c.row == first.row; });
        bool sameCol = std::all_of(openHits_.begin(), openHits_.end(), [&](Coord c) { return c.col == first.col; });
        if (sameRow || sameCol) {
            auto bounds = std::minmax_element(openHits_.begin(), openHits_.end(), [&](Coord a, Coord b) {
                return sameRow ? a.col < b.col : a.row < b.row;
            });
            Coord lo = *bounds.first, hi = *bounds.second;
            targets_.push_back(sameRow ? Coord{lo.row, lo.col - 1} : Coord{lo.row - 1, lo.col});
            targets_.push_back(sameRow ? Coord{hi.row, hi.col + 1} : Coord{hi.row + 1, hi.col});
        }
    }

    // Fallback: every neighbour of every open hit.
    for (Coord h : openHits_) {
        targets_.push_back({h.row - 1, h.col});
        targets_.push_back({h.row + 1, h.col});
        targets_.push_back({h.row, h.col - 1});
        targets_.push_back({h.row, h.col + 1});
    }
}
