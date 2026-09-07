// computer.cpp — Ahmed Hira
// Implementation of the Computer player. See computer.h.

#include <vector>
#include "computer.h"
#include "view.h"

Computer::Computer() : Player("Computer") {}

// chooseMove
// Pseudocode:
//   ask recommend() for the best move and its reason
//   print what the computer is doing and why, then return the move
Move Computer::chooseMove(const Layout& layout, int targetScore) {
    std::string reason;
    Move move = recommend(m_hand, layout, targetScore, reason);
    if (move.valid) {
        View::message("The computer placed " + move.tile.toString() + " at the "
                      + Layout::endName(move.end) + " of the layout.");
        View::message("It did so " + reason + ".");
    }
    return move;
}

// recommend
// Pseudocode:
//   list every legal (tile, end) pair
//   1. if any pair scores, take the highest score
//      (ties: prefer a double, then the heavier tile) -> reason: the points
//   2. else if any double can be placed, play the heaviest double at the end
//      that leaves the smallest open-end total -> reason: doubles are hard to
//      get rid of and count twice at the end of the round
//   3. else play the heaviest tile at the end that leaves the smallest
//      open-end total -> reason: lighten the hand for the end-of-round bonus
//      and deny the opponent a big total
Move Computer::recommend(const Hand& hand, const Layout& layout, int targetScore,
                         std::string& reason) {
    const char ends[] = { Layout::LEFT, Layout::RIGHT, Layout::UP, Layout::DOWN };
    std::vector<Move> legal;
    for (const Tile& tile : hand.getTiles()) {
        for (char end : ends) {
            if (layout.canPlace(tile, end)) {
                legal.push_back(Move(tile, end));
                if (layout.isEmpty()) {
                    break;   // an empty table: one placement per tile is enough
                }
            }
        }
    }
    if (legal.empty()) {
        reason = "because it has no legal move";
        return Move();
    }

    // 1. Best-scoring move.
    Move best = legal.front();
    int bestPoints = pointsFor(layout, best, targetScore);
    for (const Move& move : legal) {
        int points = pointsFor(layout, move, targetScore);
        bool better = points > bestPoints;
        if (points == bestPoints && points > 0) {
            if (move.tile.isDouble() && !best.tile.isDouble()) {
                better = true;
            } else if (move.tile.isDouble() == best.tile.isDouble()
                       && move.tile.getSum() > best.tile.getSum()) {
                better = true;
            }
        }
        if (better) {
            best = move;
            bestPoints = points;
        }
    }
    if (bestPoints > 0) {
        reason = "to earn a score of " + std::to_string(bestPoints);
        return best;
    }

    // 2. No scoring move: dump a double if possible, else the heaviest tile.
    bool anyDouble = false;
    for (const Move& move : legal) {
        if (move.tile.isDouble()) {
            anyDouble = true;
        }
    }
    Move pick;
    int pickSum = -1;      // open-end total left for the opponent
    for (const Move& move : legal) {
        if (anyDouble && !move.tile.isDouble()) {
            continue;
        }
        int leftover = sumAfter(layout, move);
        bool heavier = !pick.valid || move.tile.getSum() > pick.tile.getSum();
        bool sameTileBetterEnd = pick.valid && move.tile == pick.tile && leftover < pickSum;
        if (heavier || sameTileBetterEnd) {
            pick = move;
            pickSum = leftover;
        }
    }
    if (anyDouble) {
        reason = "because no move scores; doubles are hard to place later and count twice "
                 "at the end of the round, so it played its heaviest double, choosing the "
                 + Layout::endName(pick.end) + " because that leaves the smallest open-end total ("
                 + std::to_string(pickSum) + ") for the opponent";
    } else {
        reason = "because no move scores; it played its heaviest tile to lighten its hand for "
                 "the end-of-round bonus, choosing the " + Layout::endName(pick.end)
                 + " because that leaves the smallest open-end total (" + std::to_string(pickSum)
                 + ") for the opponent";
    }
    return pick;
}

// pointsFor
// Pseudocode:
//   copy the layout, place the tile, read the open-end sum
//   a multiple of the target earns that sum; anything else earns 0
int Computer::pointsFor(const Layout& layout, const Move& move, int targetScore) {
    int sum = sumAfter(layout, move);
    if (targetScore > 0 && sum % targetScore == 0) {
        return sum;
    }
    return 0;
}

// sumAfter
// Pseudocode:
//   copy the layout, place the tile, return the open-end sum
int Computer::sumAfter(const Layout& layout, const Move& move) {
    Layout trial(layout);
    trial.place(move);
    return trial.getOpenEndSum();
}
