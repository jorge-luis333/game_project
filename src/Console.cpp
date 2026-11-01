#include "Console.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace Console {

namespace {
constexpr int WIDTH = 35;      // visible width of one rendered board
const std::string GAP(6, ' ');

std::string center(const std::string& text, int width) {
    int left = (width - static_cast<int>(text.size())) / 2;
    return std::string(left, ' ') + text + std::string(width - left - text.size(), ' ');
}

bool sunkAt(const Board& board, Coord c) {
    for (const Ship& s : board.ships())
        if (s.occupies(c)) return s.isSunk();
    return false;
}

std::string cellText(const Board& board, Coord c, bool reveal, bool marked) {
    const char* color = BLUE;
    char ch = '~';
    switch (board.at(c)) {
        case Cell::Water: break;
        case Cell::Ship:  if (reveal) { color = WHITE; ch = '#'; } break;
        case Cell::Miss:  color = GREY; ch = 'o'; break;
        case Cell::Hit:   if (sunkAt(board, c)) { color = DARK_RED; ch = '#'; } else { color = RED; ch = 'X'; } break;
    }
    return std::string(color) + (marked ? INVERSE : "") + ' ' + ch + ' ' + RESET;
}

std::vector<std::string> boardLines(const Board& board, bool reveal, Coord mark) {
    std::vector<std::string> lines;
    std::string header = "    ";
    for (int c = 1; c <= Board::SIZE; ++c) header += (c < 10 ? " " : "") + std::to_string(c) + " ";
    const std::string border = "   +" + std::string(Board::SIZE * 3, '-') + "+";

    lines.push_back(GREY + header + " " + RESET);
    lines.push_back(GREY + border + RESET);
    for (int r = 0; r < Board::SIZE; ++r) {
        std::string row = std::string(GREY) + ' ' + char('A' + r) + " |" + RESET;
        for (int c = 0; c < Board::SIZE; ++c) row += cellText(board, {r, c}, reveal, mark == Coord{r, c});
        lines.push_back(row + GREY + "|" + RESET);
    }
    lines.push_back(GREY + border + RESET);
    return lines;
}

std::vector<std::string> fleetLines(const Board& board, bool reveal) {
    std::vector<std::string> lines;
    for (const Ship& s : board.ships()) {
        std::string name = s.name();
        name.resize(11, ' ');

        std::string bar;
        if (s.isSunk())  bar = DARK_RED + std::string(s.size(), 'X');
        else if (reveal) bar = RED + std::string(s.hits(), 'X') + WHITE + std::string(s.size() - s.hits(), '#');
        else             bar = GREY + std::string(s.size(), '?');

        std::string status = s.isSunk() ? "SUNK" : (reveal && s.hits() > 0) ? "DAMAGED" : "AFLOAT";
        const char* color  = s.isSunk() ? RED    : (reveal && s.hits() > 0) ? YELLOW    : GREEN;

        lines.push_back("  " + name + " " + GREY + "[" + bar + GREY + "]" + RESET +
                        std::string(7 - s.size(), ' ') + color + status + RESET +
                        std::string(12 - status.size(), ' '));
    }
    return lines;
}

void legend() {
    std::cout << "  " << BLUE << "~" << RESET << " water   " << WHITE << "#" << RESET << " ship   "
              << RED << "X" << RESET << " hit   " << DARK_RED << "#" << RESET << " sunk   "
              << GREY << "o" << RESET << " miss   " << INVERSE << " last shot " << RESET << "\n\n";
}
} // namespace

// ---- Basics ----------------------------------------------------------------

void init() {
#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(out, &mode)) SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void clear() { std::cout << "\033[2J\033[3J\033[H"; }

void pause() { readLine("  Press Enter to continue..."); }

void print(const std::string& text) { std::cout << text << '\n'; }

void print(const std::string& text, const char* color) { std::cout << color << text << RESET << '\n'; }

std::string readLine(const std::string& prompt) {
    std::cout << BOLD << prompt << RESET << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) std::exit(0);           // input closed
    size_t first = line.find_first_not_of(" \t\r");
    size_t last  = line.find_last_not_of(" \t\r");
    return first == std::string::npos ? "" : line.substr(first, last - first + 1);
}

std::string readCommand(const std::string& prompt) {
    std::string out;
    for (char ch : readLine(prompt))
        if (!std::isspace(static_cast<unsigned char>(ch))) out += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return out;
}

bool parseCoord(const std::string& text, Coord& out) {
    if (text.size() < 2 || text.size() > 3) return false;
    char letter = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    if (letter < 'A' || letter >= 'A' + Board::SIZE) return false;
    for (size_t i = 1; i < text.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    int col = std::stoi(text.substr(1));
    if (col < 1 || col > Board::SIZE) return false;
    out = {letter - 'A', col - 1};
    return true;
}

std::string toString(Coord c) {
    return std::string(1, char('A' + c.row)) + std::to_string(c.col + 1);
}

// ---- Screens ---------------------------------------------------------------

void banner() {
    std::cout << CYAN << BOLD << R"ART(
  ____    _  _____ _____ _     _____ ____  _   _ ___ ____
 | __ )  / \|_   _|_   _| |   | ____/ ___|| | | |_ _|  _ \
 |  _ \ / _ \ | |   | | | |   |  _| \___ \| |_| || || |_) |
 | |_) / ___ \| |   | | | |___| |___ ___) |  _  || ||  __/
 |____/_/   \_\_|   |_| |_____|_____|____/|_| |_|___|_|
)ART" << RESET << WHITE << R"ART(
                          __/___
                    _____/______|
            _______/_____\_______\_____
            \   o    o    o    < < <   |)ART" << BLUE << R"ART(
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
)ART" << GREY << "         Naval Command vs HunterKillerAI\n\n" << RESET;
}

void endBanner(bool won) {
    if (won) std::cout << GREEN << BOLD << R"ART(
 __     _____ ____ _____ ___  ______   __
 \ \   / /_ _/ ___|_   _/ _ \|  _ \ \ / /
  \ \ / / | | |     | || | | | |_) \ V /
   \ V /  | | |___  | || |_| |  _ < | |
    \_/  |___\____| |_| \___/|_| \_\|_|
)ART" << RESET << '\n';
    else std::cout << RED << BOLD << R"ART(
  ____  _____ _____ _____    _  _____
 |  _ \| ____|  ___| ____|  / \|_   _|
 | | | |  _| | |_  |  _|   / _ \ | |
 | |_| | |___|  _| | |___ / ___ \| |
 |____/|_____|_|   |_____/_/   \_\_|
)ART" << RESET << '\n';
}

void title(const std::string& text) {
    const std::string line(76, '=');
    std::cout << CYAN << line << '\n' << BOLD << "  " << text << RESET << '\n'
              << CYAN << line << RESET << "\n\n";
}

void draw(const Board& board, const std::string& heading) {
    std::cout << BOLD << CYAN << center(heading, WIDTH) << RESET << '\n';
    for (const std::string& line : boardLines(board, true, {-1, -1})) std::cout << line << '\n';
    std::cout << '\n';
    legend();
}

void draw(const Board& own, const Board& enemy, bool revealEnemy, Coord markOwn, Coord markEnemy) {
    std::cout << BOLD << CYAN << center("YOUR FLEET", WIDTH) << GAP << center("ENEMY WATERS", WIDTH) << RESET << '\n';

    auto left = boardLines(own, true, markOwn);
    auto right = boardLines(enemy, revealEnemy, markEnemy);
    for (size_t i = 0; i < left.size(); ++i) std::cout << left[i] << GAP << right[i] << '\n';
    std::cout << '\n';

    auto ownFleet = fleetLines(own, true);
    auto enemyFleet = fleetLines(enemy, revealEnemy);
    for (size_t i = 0; i < ownFleet.size() || i < enemyFleet.size(); ++i)
        std::cout << (i < ownFleet.size() ? ownFleet[i] : std::string(WIDTH, ' ')) << GAP
                  << (i < enemyFleet.size() ? enemyFleet[i] : "") << '\n';
    std::cout << '\n';
    legend();
}

} // namespace Console
