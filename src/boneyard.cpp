// boneyard.cpp — Ahmed Hira
// Implementation of the Boneyard class. See boneyard.h for the class layout.

#include <algorithm>
#include <random>
#include "boneyard.h"

// Default constructor
// Pseudocode:
//   start with no tiles
Boneyard::Boneyard() : m_tiles() {}

// Copy constructor
// Pseudocode:
//   copy the other boneyard's tile list
Boneyard::Boneyard(const Boneyard& other) : m_tiles(other.m_tiles) {}

// Copy assignment
// Pseudocode:
//   copy the other boneyard's tile list
Boneyard& Boneyard::operator=(const Boneyard& other) {
    m_tiles = other.m_tiles;
    return *this;
}

// peekFront
// Pseudocode:
//   if empty, return the blank tile
//   otherwise return a copy of the first tile
Tile Boneyard::peekFront() const {
    if (isEmpty()) {
        return Tile();
    }
    return m_tiles.front();
}

// getTiles
// Pseudocode:
//   return a copy of the whole tile list
std::vector<Tile> Boneyard::getTiles() const {
    return m_tiles;
}

// contains
// Pseudocode:
//   true if any tile in the list == the target
bool Boneyard::contains(const Tile& tile) const {
    for (const Tile& t : m_tiles) {
        if (t == tile) {
            return true;
        }
    }
    return false;
}

// toString
// Pseudocode:
//   join each tile's text with single spaces
std::string Boneyard::toString() const {
    std::string text;
    for (int i = 0; i < size(); i++) {
        if (i > 0) {
            text += " ";
        }
        text += m_tiles[i].toString();
    }
    return text;
}

// reset
// Pseudocode:
//   clear the list
//   for left = 0..6
//     for right = left..6
//       add tile left-right
//   (this produces each combination exactly once: 28 tiles)
void Boneyard::reset() {
    m_tiles.clear();
    for (int left = Tile::MIN_PIPS; left <= Tile::MAX_PIPS; left++) {
        for (int right = left; right <= Tile::MAX_PIPS; right++) {
            m_tiles.push_back(Tile(left, right));
        }
    }
}

// shuffle
// Pseudocode:
//   seed a random engine from the system and shuffle the list in place
void Boneyard::shuffle() {
    std::random_device seed;
    std::mt19937 engine(seed());
    std::shuffle(m_tiles.begin(), m_tiles.end(), engine);
}

// drawTile
// Pseudocode:
//   if empty, return false
//   copy the front tile into drawnTile, remove it from the list, return true
bool Boneyard::drawTile(Tile& drawnTile) {
    if (isEmpty()) {
        return false;
    }
    drawnTile = m_tiles.front();
    m_tiles.erase(m_tiles.begin());
    return true;
}

// addToBottom
// Pseudocode:
//   if the tile is already here, return false
//   otherwise append it to the end and return true
bool Boneyard::addToBottom(Tile tile) {
    if (contains(tile)) {
        return false;
    }
    m_tiles.push_back(tile);
    return true;
}

// setTiles
// Pseudocode:
//   if the new list has duplicates, return false
//   otherwise replace the contents with a copy of the list and return true
bool Boneyard::setTiles(const std::vector<Tile>& tiles) {
    if (hasDuplicates(tiles)) {
        return false;
    }
    m_tiles = tiles;
    return true;
}

// clear
// Pseudocode:
//   empty the tile list
void Boneyard::clear() {
    m_tiles.clear();
}

// hasDuplicates
// Pseudocode:
//   for each pair of positions i < j, if the tiles are equal return true
//   return false if no pair matched
bool Boneyard::hasDuplicates(const std::vector<Tile>& tiles) {
    for (size_t i = 0; i < tiles.size(); i++) {
        for (size_t j = i + 1; j < tiles.size(); j++) {
            if (tiles[i] == tiles[j]) {
                return true;
            }
        }
    }
    return false;
}
