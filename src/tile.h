// tile.h — Ahmed Hira
// A single domino tile: two ends, each showing 0-6 pips.
// Model class only: no input/output happens here.

#ifndef TILE_H
#define TILE_H

#include <string>

class Tile {
public:
    // ----- constants -----
    static const int MIN_PIPS = 0;
    static const int MAX_PIPS = 6;

    // ----- constructors -----
    // Default constructor gives the blank tile 0-0.
    // If either value is out of range the tile stays 0-0.
    Tile(int leftPips = MIN_PIPS, int rightPips = MIN_PIPS);

    // ----- selectors -----
    inline int getLeftPips() const { return m_leftPips; }
    inline int getRightPips() const { return m_rightPips; }

    // Total pips on the tile (decides who goes first, and used for scoring).
    inline int getSum() const { return m_leftPips + m_rightPips; }

    // True when both ends match, e.g. 3-3.
    inline bool isDouble() const { return m_leftPips == m_rightPips; }

    // A copy of this tile with the ends swapped, e.g. 2-5 -> 5-2.
    Tile flipped() const;

    // Text form for display and for the save file, e.g. "2-5".
    std::string toString() const;

    // Two tiles are the same tile even if one is flipped: 2-5 == 5-2.
    bool operator==(const Tile& other) const;

    // ----- mutators -----
    // Change both ends. Returns false and leaves the tile unchanged
    // if either value is outside MIN_PIPS..MAX_PIPS.
    bool setPips(int leftPips, int rightPips);

    // ----- utility -----
    // True if the value is a legal number of pips for one end.
    static bool isValidPips(int pips);

    // Parse text like "2-5" into a tile. Returns false (and leaves
    // result alone) if the text is not two legal pip counts joined by '-'.
    static bool fromString(const std::string& text, Tile& result);

private:
    int m_leftPips;
    int m_rightPips;
};

#endif
