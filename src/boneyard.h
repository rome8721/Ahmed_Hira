// boneyard.h — Ahmed Hira
// The shuffled stock of tiles not yet dealt. Tiles are drawn from the
// front (left-most in the save file) and returned to the bottom (right end).
// Model class only: no input/output happens here.

#ifndef BONEYARD_H
#define BONEYARD_H

#include <string>
#include <vector>
#include "tile.h"

class Boneyard {
public:
    // ----- constants -----
    // A double-six set has 28 tiles: 7 + 6 + 5 + 4 + 3 + 2 + 1.
    static const int FULL_SET_SIZE = 28;

    // ----- constructors -----
    // Default constructor: an empty boneyard. Call reset() for a full set.
    Boneyard();

    // Copy constructor: copies every tile (Boneyard holds an aggregate).
    Boneyard(const Boneyard& other);

    // Copy assignment, for the same reason.
    Boneyard& operator=(const Boneyard& other);

    // ----- selectors -----
    inline int size() const { return static_cast<int>(m_tiles.size()); }
    inline bool isEmpty() const { return m_tiles.empty(); }

    // Copy of the tile that the next draw would take. Blank tile if empty.
    Tile peekFront() const;

    // Copy of every tile, front to back.
    std::vector<Tile> getTiles() const;

    // True if this tile (either orientation) is still in the boneyard.
    bool contains(const Tile& tile) const;

    // Space-separated text in save-file form, e.g. "0-6 1-5 5-6".
    std::string toString() const;

    // ----- mutators -----
    // Fill with the full double-six set in order 0-0, 0-1, ... 6-6.
    void reset();

    // Randomly reorder the tiles.
    void shuffle();

    // Take the front tile. Returns false (and leaves drawnTile alone) if empty.
    bool drawTile(Tile& drawnTile);

    // Put a tile on the bottom. Returns false if that tile is already here.
    bool addToBottom(Tile tile);

    // Replace the contents (used when resuming from a file).
    // Returns false and leaves the boneyard unchanged if the list has duplicates.
    bool setTiles(const std::vector<Tile>& tiles);

    // Remove every tile.
    void clear();

private:
    std::vector<Tile> m_tiles;

    // ----- utility -----
    // True if the list contains the same tile twice (either orientation).
    static bool hasDuplicates(const std::vector<Tile>& tiles);
};

#endif
