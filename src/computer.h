// computer.h — Ahmed Hira
// The computer player. Its strategy lives in recommend() so the Human
// player's help mode can reuse the exact same reasoning.

#ifndef COMPUTER_H
#define COMPUTER_H

#include <string>
#include "player.h"

class Computer : public Player {
public:
    Computer();

    // Pick a move using recommend() and announce the tile, place, and reason.
    Move chooseMove(const Layout& layout, int targetScore) override;

    // ----- strategy (shared with Human's help mode) -----
    // Best move for this hand on this layout. Fills reason with a sentence
    // explaining the choice. Returns an invalid Move if nothing can be placed.
    static Move recommend(const Hand& hand, const Layout& layout, int targetScore,
                          std::string& reason);

private:
    // Points a move would earn: the open-end sum if it is a multiple of the target, else 0.
    static int pointsFor(const Layout& layout, const Move& move, int targetScore);

    // Open-end sum after a move (what the opponent would face).
    static int sumAfter(const Layout& layout, const Move& move);
};

#endif
