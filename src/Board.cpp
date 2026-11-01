#include "Board.h"
#include <random>

int randomInt(int lo, int hi) {
    static std::mt19937 gen{std::random_device{}()};
    return std::uniform_int_distribution<int>(lo, hi)(gen);
}

// ---- Special member functions ---------------------------------------------

Board::Board() : cells_(new Cell[SIZE * SIZE]) {
    std::fill(cells_, cells_ + SIZE * SIZE, Cell::Water);
}

Board::~Board() {
    delete[] cells_;
}

Board::Board(const Board& other) : cells_(new Cell[SIZE * SIZE]), ships_(other.ships_) {
    std::copy(other.cells_, other.cells_ + SIZE * SIZE, cells_);   // deep copy
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        if (!cells_) cells_ = new Cell[SIZE * SIZE];                 // we may have been moved from
        std::copy(other.cells_, other.cells_ + SIZE * SIZE, cells_);
        ships_ = other.ships_;
    }
    return *this;
}

Board::Board(Board&& other) noexcept : cells_(other.cells_), ships_(std::move(other.ships_)) {
    other.cells_ = nullptr;                                          // steal the array
}

Board& Board::operator=(Board&& other) noexcept {
    if (this != &other) {
        delete[] cells_;
        cells_ = other.cells_;
        other.cells_ = nullptr;
        ships_ = std::move(other.ships_);
    }
    return *this;
}

// ---- Setup -----------------------------------------------------------------

std::vector<Ship> Board::standardFleet() {
    return { {"Carrier", 5}, {"Battleship", 4}, {"Cruiser", 3}, {"Submarine", 3}, {"Destroyer", 2} };
}

Board Board::randomFleet() {
    Board board;
    for (const Ship& ship : standardFleet()) {
        bool placed = false;
        while (!placed)
            placed = board.placeShip(ship, {randomInt(0, SIZE - 1), randomInt(0, SIZE - 1)}, randomInt(0, 1) == 0);
    }
    return board;
}

bool Board::inBounds(Coord c) {
    return c.row >= 0 && c.row < SIZE && c.col >= 0 && c.col < SIZE;
}

bool Board::canPlace(const Ship& ship, Coord start, bool horizontal) const {
    Ship probe = ship;
    probe.place(start, horizontal);
    return std::all_of(probe.cells().begin(), probe.cells().end(),
                       [this](Coord c) { return inBounds(c) && at(c) == Cell::Water; });
}

bool Board::placeShip(Ship ship, Coord start, bool horizontal) {
    if (!canPlace(ship, start, horizontal)) return false;
    ship.place(start, horizontal);
    for (Coord c : ship.cells()) cell(c) = Cell::Ship;
    ships_.push_back(std::move(ship));
    return true;
}

// ---- Combat ----------------------------------------------------------------

Shot Board::fire(Coord target) {
    if (!inBounds(target)) return {ShotResult::Invalid};
    Cell& c = cell(target);
    if (c == Cell::Hit || c == Cell::Miss) return {ShotResult::Repeat};
    if (c == Cell::Water) { c = Cell::Miss; return {ShotResult::Miss}; }

    c = Cell::Hit;
    for (Ship& ship : ships_) {
        if (ship.occupies(target)) {
            ship.registerHit();
            return {ship.isSunk() ? ShotResult::Sunk : ShotResult::Hit, &ship};
        }
    }
    return {ShotResult::Hit};
}

Shot Board::fire(int row, int col) {
    return fire(Coord{row, col});
}

bool Board::allSunk() const {
    return std::all_of(ships_.begin(), ships_.end(), [](const Ship& s) { return s.isSunk(); });
}
