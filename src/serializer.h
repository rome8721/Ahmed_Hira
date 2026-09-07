// serializer.h — Ahmed Hira
// Reads and writes the course's text save-file format.

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string>
#include "gamestate.h"

class Serializer {
public:
    // Write the state to the file. Returns false if the file cannot be opened.
    static bool save(const GameState& state, const std::string& fileName);

    // Read the file into state. Returns false (state unspecified) on any problem.
    static bool load(const std::string& fileName, GameState& state);

private:
    // Fill a hand from text like "0-1 2-5 4-4". False on a bad or repeated tile.
    static bool parseHand(const std::string& text, Hand& hand);

    // Copy of text with leading/trailing spaces removed.
    static std::string trim(const std::string& text);

    // If line begins with prefix, put the remainder (trimmed) in rest and return true.
    static bool startsWith(const std::string& line, const std::string& prefix, std::string& rest);
};

#endif
