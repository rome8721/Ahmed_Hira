// hand.h — Ahmed Hira
// The tiles one player is currently holding.
// Model class only: no input/output happens here.

#ifndef HAND_H
#define HAND_H

#include <string>
#include <vector>
#include "tile.h"

class Hand {
public:
    // ----- constructors -----
    // Default constructor: an empty hand.
    Hand();

    // Copy constructor: copies every tile (Hand holds an aggregate).
    Hand(const Hand& other);

    // Copy assignment, for the same reason.
    Hand& operator=(const Hand& other);

    // ----- selectors -----
    inline int size() const { return static_cast<int>(m_tiles.size()); }
    inline bool isEmpty() const { return m_tiles.empty(); }

    // Copy of the tile at a position, 0-based. Returns the blank tile 0-0
    // if the index is out of range.
    Tile getTile(int index) const;

    // Copy of every tile, in hand order.
    std::vector<Tile> getTiles() const;

    // Total pips of all tiles (used for the end-of-round bonus).
    int getSum() const;

    // True if this exact tile (either orientation) is in the hand.
    bool contains(const Tile& tile) const;

    // Space-separated text in save-file form, e.g. "0-1 2-5 4-4".
    std::string toString() const;

    // ----- mutators -----
    // Add a tile. Returns false if the tile is already in the hand.
    bool addTile(Tile tile);

    // Remove a tile. Returns false if the tile is not in the hand.
    bool removeTile(Tile tile);

    // Remove every tile.
    void clear();

private:
    std::vector<Tile> m_tiles;

    // ----- utility -----
    // Position of a tile in m_tiles, or NOT_FOUND if absent.
    static const int NOT_FOUND = -1;
    int indexOf(const Tile& tile) const;
};

#endif
