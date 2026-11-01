# Battleship – Console Naval Battle

A terminal Battleship game in C++17: you against **HunterKillerAI**.

## Build & run

```sh
# g++ / MinGW / clang
g++ -std=c++17 -O2 src/*.cpp -o battleship
./battleship

# or CMake (also works with Visual Studio)
cmake -B build && cmake --build build
```

Colours use ANSI escape codes (turned on automatically in the Windows 10+ console).

## Project map (read in this order)

| File | What it holds | Lines |
|---|---|---|
| `src/Ship.h` | `Coord` + `Ship` (name, size, cells, hits) | ~40 |
| `src/Board.h/.cpp` | 10x10 grid, ship placement, `fire()`, **Rule of Five** | ~150 |
| `src/Player.h/.cpp` | `Player` → `HumanPlayer`, `ComputerPlayer` → `HunterKillerAI` | ~235 |
| `src/Console.h/.cpp` | All input/output: colours, board drawing, prompts | ~245 |
| `src/Game.h/.cpp` | Menu, turn loop, battle log, game-over screen | ~150 |
| `src/main.cpp` | `Game().run()` | 7 |

Dependencies go one way: `Game → Player → Board → Ship`, and `Console` draws the boards.
The rules (`Ship`, `Board`) never do I/O.

## Where each OOP requirement lives

| Requirement | Where |
|---|---|
| **Encapsulation** `public` / `protected` / `private` | `Player`: public interface, `protected` `name_`/`board_` (used by subclasses), `private` `shots_`/`hits_` (changed only by `Player::attack`). `Ship` and `Board` keep all data `private`. |
| **Inheritance (depth 3)** | `Player` → `ComputerPlayer` → `HunterKillerAI` (plus `Player` → `HumanPlayer`) in `Player.h` |
| **Composition** | `Player` *has a* `Board`; `Board` *has* `std::vector<Ship>` |
| **Polymorphism: overloading** | `Board::fire(Coord)` / `Board::fire(int,int)`; `Console::print(text)` / `print(text, color)`; `Console::draw(board, heading)` / `draw(own, enemy, …)` |
| **Polymorphism: virtual** | `Player::chooseTarget` / `onShotResult` overridden per player; `Game` holds the AI as `std::unique_ptr<Player>` |
| **Special member functions** | `Board` owns a raw `Cell*` array, so it defines all six (`Board.cpp`), and the game uses each one: |
| · default constructor | `Board()` – fills the grid with water |
| · destructor | `~Board()` – `delete[]` the grid |
| · copy constructor | `history.push_back(board_)` – undo snapshot during manual placement |
| · copy assignment | `board_ = history.back()` – the undo itself |
| · move constructor | `Player(name, Board board) : board_(std::move(board))` |
| · move assignment | `board_ = Board::randomFleet()` – auto-deploy / re-roll |

## HunterKillerAI

- **Hunt mode:** fires at random untried cells on a checkerboard (every ship covers at least one of them).
- **Kill mode:** after a hit, it queues the neighbouring cells. Once two hits line up, it tries both ends of the line first.
  When the ship sinks it removes those cells and goes back to hunting.
- It only reads hit/miss marks on your board and never looks at hidden ships.

## How to play

1. Choose **New Game**, enter your name, and deploy your fleet: auto (re-roll as often as you like) or manual (`A1 H`, `C5 V`, `U` = undo).
2. Each turn, type a target like `B7`. The AI fires back, and the battle log shows what happened.
3. Sink all five enemy ships (Carrier 5, Battleship 4, Cruiser 3, Submarine 3, Destroyer 2) to win.
