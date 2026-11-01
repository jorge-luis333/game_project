#pragma once
#include "Ship.h"

enum class Cell { Water, Ship, Hit, Miss };
enum class ShotResult { Invalid, Repeat, Miss, Hit, Sunk };

struct Shot {
    ShotResult result;
    const Ship* ship = nullptr;   // the ship that was hit (nullptr on a miss)
};

int randomInt(int lo, int hi);    // shared random generator, inclusive range

// 10x10 grid + the ships on it.
// Owns a raw heap array, so it implements all special member functions (Rule of Five).
class Board {
public:
    static constexpr int SIZE = 10;

    Board();                                   // default constructor
    ~Board();                                  // destructor
    Board(const Board& other);                 // copy constructor
    Board& operator=(const Board& other);      // copy assignment
    Board(Board&& other) noexcept;             // move constructor
    Board& operator=(Board&& other) noexcept;  // move assignment

    static std::vector<Ship> standardFleet();
    static Board randomFleet();
    static bool inBounds(Coord c);

    bool canPlace(const Ship& ship, Coord start, bool horizontal) const;
    bool placeShip(Ship ship, Coord start, bool horizontal);

    Shot fire(Coord target);                   // overloads (compile-time polymorphism)
    Shot fire(int row, int col);

    Cell at(Coord c) const { return cells_[c.row * SIZE + c.col]; }
    bool wasShot(Coord c) const { return at(c) == Cell::Hit || at(c) == Cell::Miss; }
    bool allSunk() const;
    const std::vector<Ship>& ships() const { return ships_; }

private:
    Cell& cell(Coord c) { return cells_[c.row * SIZE + c.col]; }

    Cell* cells_;                              // SIZE*SIZE cells on the heap
    std::vector<Ship> ships_;                  // composition: Board HAS Ships
};
