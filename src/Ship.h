#pragma once
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

// A grid position. row 0..9 = 'A'..'J', col 0..9 = '1'..'10'.
struct Coord {
    int row = 0;
    int col = 0;
    bool operator==(const Coord& other) const { return row == other.row && col == other.col; }
};

// A single ship. Pure data + rules, no I/O.
class Ship {
public:
    Ship(std::string name, int size) : name_(std::move(name)), size_(size) {}

    const std::string& name() const { return name_; }
    int  size() const { return size_; }
    int  hits() const { return hits_; }
    bool isSunk() const { return hits_ >= size_; }
    const std::vector<Coord>& cells() const { return cells_; }

    void place(Coord start, bool horizontal) {
        cells_.clear();
        for (int i = 0; i < size_; ++i)
            cells_.push_back(horizontal ? Coord{start.row, start.col + i} : Coord{start.row + i, start.col});
    }

    bool occupies(Coord c) const { return std::find(cells_.begin(), cells_.end(), c) != cells_.end(); }
    void registerHit() { if (!isSunk()) ++hits_; }

private:                     // encapsulation: state only changes through the methods above
    std::string name_;
    int size_;
    int hits_ = 0;
    std::vector<Coord> cells_;
};
