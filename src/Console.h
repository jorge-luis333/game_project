#pragma once
#include "Board.h"
#include <string>

// All terminal input/output lives here.
namespace Console {
    inline constexpr const char* RESET    = "\033[0m";
    inline constexpr const char* BOLD     = "\033[1m";
    inline constexpr const char* INVERSE  = "\033[7m";
    inline constexpr const char* RED      = "\033[91m";
    inline constexpr const char* DARK_RED = "\033[31m";
    inline constexpr const char* GREEN    = "\033[92m";
    inline constexpr const char* YELLOW   = "\033[93m";
    inline constexpr const char* BLUE     = "\033[34m";
    inline constexpr const char* CYAN     = "\033[96m";
    inline constexpr const char* MAGENTA  = "\033[95m";
    inline constexpr const char* WHITE    = "\033[97m";
    inline constexpr const char* GREY     = "\033[90m";

    void init();                       // enables ANSI colours on Windows
    void clear();
    void pause();

    void print(const std::string& text);                      // overloads
    void print(const std::string& text, const char* color);

    std::string readLine(const std::string& prompt);          // trimmed raw text
    std::string readCommand(const std::string& prompt);       // uppercase, no spaces

    bool parseCoord(const std::string& text, Coord& out);     // "B7" -> {1, 6}
    std::string toString(Coord c);                            // {1, 6} -> "B7"

    void banner();
    void endBanner(bool won);
    void title(const std::string& text);

    void draw(const Board& board, const std::string& heading);              // overloads
    void draw(const Board& own, const Board& enemy, bool revealEnemy,
              Coord markOwn, Coord markEnemy);
}
