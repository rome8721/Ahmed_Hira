// tile.cpp — Ahmed Hira
// Implementation of the Tile class. See tile.h for the class layout.

#include <cctype>
#include "tile.h"

// Constructor
// Pseudocode:
//   start as the blank tile 0-0
//   try to set the requested pips; if they are invalid, stay 0-0
Tile::Tile(int leftPips, int rightPips) : m_leftPips(MIN_PIPS), m_rightPips(MIN_PIPS) {
    setPips(leftPips, rightPips);
}

// flipped
// Pseudocode:
//   return a new tile with left and right swapped
Tile Tile::flipped() const {
    return Tile(m_rightPips, m_leftPips);
}

// toString
// Pseudocode:
//   join left pips, a dash, and right pips into one string
std::string Tile::toString() const {
    return std::to_string(m_leftPips) + "-" + std::to_string(m_rightPips);
}

// operator==
// Pseudocode:
//   same tile if both ends match as-is, OR both ends match when flipped
bool Tile::operator==(const Tile& other) const {
    bool sameWay = (m_leftPips == other.m_leftPips && m_rightPips == other.m_rightPips);
    bool flippedWay = (m_leftPips == other.m_rightPips && m_rightPips == other.m_leftPips);
    return sameWay || flippedWay;
}

// setPips
// Pseudocode:
//   if either value is out of range, return false without changing anything
//   otherwise store both values and return true
bool Tile::setPips(int leftPips, int rightPips) {
    if (!isValidPips(leftPips) || !isValidPips(rightPips)) {
        return false;
    }
    m_leftPips = leftPips;
    m_rightPips = rightPips;
    return true;
}

// isValidPips
// Pseudocode:
//   legal when MIN_PIPS <= pips <= MAX_PIPS
bool Tile::isValidPips(int pips) {
    return pips >= MIN_PIPS && pips <= MAX_PIPS;
}

// fromString
// Pseudocode:
//   text must be exactly "a-b" where a and b are single digits
//   convert both digits; use setPips on a scratch tile so the range check applies
//   on success copy the scratch tile into result
bool Tile::fromString(const std::string& text, Tile& result) {
    const size_t EXPECTED_LENGTH = 3;   // digit, dash, digit
    if (text.length() != EXPECTED_LENGTH || text[1] != '-') {
        return false;
    }
    if (!isdigit(text[0]) || !isdigit(text[2])) {
        return false;
    }
    Tile scratch;
    if (!scratch.setPips(text[0] - '0', text[2] - '0')) {
        return false;
    }
    result = scratch;
    return true;
}
