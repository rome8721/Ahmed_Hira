// move.h — Ahmed Hira
// One placement: which tile goes on which open end of the layout.
// A plain record shared by the players, the layout, and the round.

#ifndef MOVE_H
#define MOVE_H

#include "tile.h"

struct Move {
    Tile tile;          // the tile to place
    char end;           // 'L', 'R', 'U', or 'D'
    bool valid;         // false means "no move" (e.g. nothing can be placed)

    Move() : tile(), end('L'), valid(false) {}
    Move(Tile t, char e) : tile(t), end(e), valid(true) {}
};

#endif
