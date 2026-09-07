// hand.cpp — Ahmed Hira
// Implementation of the Hand class. See hand.h for the class layout.

#include "hand.h"

// Default constructor
// Pseudocode:
//   start with no tiles (the vector is empty by default)
Hand::Hand() : m_tiles() {}

// Copy constructor
// Pseudocode:
//   copy the other hand's tile list
Hand::Hand(const Hand& other) : m_tiles(other.m_tiles) {}

// Copy assignment
// Pseudocode:
//   copy the other hand's tile list (self-assignment is harmless here)
Hand& Hand::operator=(const Hand& other) {
    m_tiles = other.m_tiles;
    return *this;
}

// getTile
// Pseudocode:
//   if index is outside 0..size-1, return the blank tile
//   otherwise return a copy of the tile at that index
Tile Hand::getTile(int index) const {
    if (index < 0 || index >= size()) {
        return Tile();
    }
    return m_tiles[index];
}

// getTiles
// Pseudocode:
//   return a copy of the whole tile list
std::vector<Tile> Hand::getTiles() const {
    return m_tiles;
}

// getSum
// Pseudocode:
//   total = 0
//   for each tile, add its pip sum to total
//   return total
int Hand::getSum() const {
    int total = 0;
    for (const Tile& tile : m_tiles) {
        total += tile.getSum();
    }
    return total;
}

// contains
// Pseudocode:
//   true when indexOf finds the tile
bool Hand::contains(const Tile& tile) const {
    return indexOf(tile) != NOT_FOUND;
}

// toString
// Pseudocode:
//   join each tile's text with single spaces
std::string Hand::toString() const {
    std::string text;
    for (int i = 0; i < size(); i++) {
        if (i > 0) {
            text += " ";
        }
        text += m_tiles[i].toString();
    }
    return text;
}

// addTile
// Pseudocode:
//   if the tile is already here, return false
//   otherwise append it and return true
bool Hand::addTile(Tile tile) {
    if (contains(tile)) {
        return false;
    }
    m_tiles.push_back(tile);
    return true;
}

// removeTile
// Pseudocode:
//   find the tile's position
//   if not found, return false
//   otherwise erase it and return true
bool Hand::removeTile(Tile tile) {
    int position = indexOf(tile);
    if (position == NOT_FOUND) {
        return false;
    }
    m_tiles.erase(m_tiles.begin() + position);
    return true;
}

// clear
// Pseudocode:
//   empty the tile list
void Hand::clear() {
    m_tiles.clear();
}

// indexOf
// Pseudocode:
//   walk the list; return the first position whose tile == the target
//   return NOT_FOUND if the loop finishes
int Hand::indexOf(const Tile& tile) const {
    for (int i = 0; i < size(); i++) {
        if (m_tiles[i] == tile) {
            return i;
        }
    }
    return NOT_FOUND;
}
