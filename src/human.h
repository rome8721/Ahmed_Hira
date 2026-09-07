// human.h — Ahmed Hira
// The human player. Reads the move from the keyboard (validated) and can
// ask the Computer's strategy for a recommendation.

#ifndef HUMAN_H
#define HUMAN_H

#include "player.h"

class Human : public Player {
public:
    Human();

    // Ask the user for a tile and an end until the choice is legal.
    // Typing "help" prints the computer's recommendation.
    Move chooseMove(const Layout& layout, int targetScore) override;

private:
    // Ask for one open end letter; returns 'L' without asking on an empty table.
    char askEnd(const Layout& layout, const Tile& tile) const;
};

#endif
