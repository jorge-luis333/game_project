#pragma once
#include "Board.h"
#include <deque>
#include <string>

// Inheritance (depth 3):  Player -> ComputerPlayer -> HunterKillerAI
//                         Player -> HumanPlayer
// Composition:            Player HAS-A Board, Board HAS Ships
class Player {
public:
    Player(std::string name, Board board);
    virtual ~Player() = default;

    virtual Coord chooseTarget(const Board& enemy) = 0;        // runtime polymorphism
    virtual void  onShotResult(Coord /*target*/, const Shot& /*shot*/) {}

    Shot attack(Player& enemy, Coord target);                  // fire + bookkeeping

    const std::string& name() const { return name_; }
    const Board& board() const { return board_; }
    bool hasLost() const { return board_.allSunk(); }
    int  shots() const { return shots_; }
    int  hits() const { return hits_; }
    int  accuracy() const { return shots_ ? hits_ * 100 / shots_ : 0; }

protected:                  // derived classes may use these directly
    std::string name_;
    Board board_;

private:                    // only Player itself updates the stats
    int shots_ = 0;
    int hits_ = 0;
};

class HumanPlayer : public Player {
public:
    explicit HumanPlayer(std::string name);
    void  deployFleet();                                       // interactive placement
    Coord chooseTarget(const Board& enemy) override;           // reads keyboard

private:
    void autoDeploy();
    void manualDeploy();
};

class ComputerPlayer : public Player {
public:
    explicit ComputerPlayer(std::string name);
    Coord chooseTarget(const Board& enemy) override;           // random untried cell

protected:
    Coord randomUntried(const Board& enemy, bool checkerboard) const;
};

// Hunt mode: checkerboard sweep. Kill mode: chase hits until the ship sinks.
class HunterKillerAI : public ComputerPlayer {
public:
    HunterKillerAI();
    Coord chooseTarget(const Board& enemy) override;
    void  onShotResult(Coord target, const Shot& shot) override;

private:
    void rebuildTargets();
    std::vector<Coord> openHits_;   // hits on ships that are not sunk yet
    std::deque<Coord>  targets_;    // cells to try next (kill mode)
};
