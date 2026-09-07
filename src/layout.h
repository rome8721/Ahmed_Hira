// layout.h — Ahmed Hira
// The tiles on the table. A main line runs left (L) to right (R). The first
// double played is the spinner; it also opens an up (U) and a down (D) arm.
// Tiles are stored oriented so that touching pips match:
//   main line:  m_line[i].right == m_line[i+1].left
//   arms:       arm[0].left == spinner pip, arm[i].right is the open end
// Model class only: no input/output happens here.

#ifndef LAYOUT_H
#define LAYOUT_H

#include <string>
#include <vector>
#include "tile.h"
#include "move.h"

class Layout {
public:
    // ----- constants -----
    static const char LEFT  = 'L';
    static const char RIGHT = 'R';
    static const char UP    = 'U';
    static const char DOWN  = 'D';
    static const int NO_SPINNER = -1;

    // ----- constructors -----
    // Default constructor: an empty table.
    Layout();

    // Copy constructor: copies every tile (Layout holds aggregates).
    Layout(const Layout& other);

    // Copy assignment, for the same reason.
    Layout& operator=(const Layout& other);

    // ----- selectors -----
    inline bool isEmpty() const { return m_line.empty(); }
    inline bool hasSpinner() const { return m_spinnerIndex != NO_SPINNER; }

    // Copies of the tiles, for display. Arms run from the spinner outward.
    std::vector<Tile> getLine() const;
    std::vector<Tile> getUpArm() const;
    std::vector<Tile> getDownArm() const;
    inline int getSpinnerIndex() const { return m_spinnerIndex; }

    // True if tiles may currently be placed at this end.
    bool isOpenEnd(char end) const;

    // Pips showing at an open end. Returns NO_SPINNER (-1) if the end is closed.
    int getEndPips(char end) const;

    // Sum of pips at all open ends, with doubles counting both pips.
    int getOpenEndSum() const;

    // True if this tile may legally be placed at this end right now.
    bool canPlace(const Tile& tile, char end) const;
    inline bool canPlace(const Move& move) const { return move.valid && canPlace(move.tile, move.end); }

    // Save-file form, e.g. "L 5-3 [D 3-3 U] 3-2 2-2 R".
    std::string toString() const;

    // ----- mutators -----
    // Place a tile. Returns false and changes nothing if the move is illegal.
    bool place(Tile tile, char end);
    inline bool place(const Move& move) { return move.valid && place(move.tile, move.end); }

    // Rebuild from save-file text (the part between and including L and R).
    // Returns false and changes nothing if the text is not a valid layout.
    bool fromString(const std::string& text);

    // Remove every tile.
    void clear();

    // ----- utility -----
    // "Left", "Right", "Up", or "Down" for the end letter.
    static std::string endName(char end);

    // True for one of the four end letters.
    static bool isEndLetter(char end);

private:
    std::vector<Tile> m_line;
    std::vector<Tile> m_up;
    std::vector<Tile> m_down;
    int m_spinnerIndex;

    // Pips of the spinner (either end; it is a double).
    int spinnerPips() const;

    // Pips a tile shows at an open end: both pips when it is a double.
    static int endValue(const Tile& tile, int exposedPips);

    // Flip tiles as needed so that tiles[i].right == tiles[i+1].left.
    // Returns false if no orientation makes the chain connect.
    static bool orientChain(std::vector<Tile>& tiles);
};

#endif
