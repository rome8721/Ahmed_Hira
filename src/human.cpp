// human.cpp — Ahmed Hira
// Implementation of the Human player. See human.h.

#include <cctype>
#include "human.h"
#include "computer.h"
#include "view.h"

Human::Human() : Player("Human") {}

// chooseMove
// Pseudocode:
//   repeat:
//     ask for a tile ("2-5") or "help"
//     help -> print the computer's recommendation and its reason, ask again
//     bad text or a tile not in the hand -> explain, ask again
//     ask for the end; if the layout refuses the placement -> explain, ask again
//     otherwise return the move
Move Human::chooseMove(const Layout& layout, int targetScore) {
    while (true) {
        std::string answer = View::readWord(
            "Enter the tile to play (like 2-5), or 'help' for a suggestion: ");
        if (answer == "help" || answer == "h") {
            std::string reason;
            Move suggestion = Computer::recommend(m_hand, layout, targetScore, reason);
            if (suggestion.valid) {
                View::message("  Suggestion: place " + suggestion.tile.toString() + " at the "
                              + Layout::endName(suggestion.end) + ", " + reason + ".");
            } else {
                View::message("  Suggestion: nothing in your hand can be placed.");
            }
            continue;
        }
        Tile tile;
        if (!Tile::fromString(answer, tile)) {
            View::message("  That is not a tile. Type two pip counts joined by a dash, like 2-5.");
            continue;
        }
        if (!m_hand.contains(tile)) {
            View::message("  " + tile.toString() + " is not in your hand.");
            continue;
        }
        char end = askEnd(layout, tile);
        if (!layout.canPlace(tile, end)) {
            View::message("  " + tile.toString() + " does not match the "
                          + std::to_string(layout.getEndPips(end)) + " at the "
                          + Layout::endName(end) + " end.");
            continue;
        }
        return Move(tile, end);
    }
}

// askEnd
// Pseudocode:
//   empty table -> no choice to make, return L
//   list the open ends; repeat until the user types one of them
char Human::askEnd(const Layout& layout, const Tile& tile) const {
    if (layout.isEmpty()) {
        return Layout::LEFT;
    }
    std::string open;
    const char ends[] = { Layout::LEFT, Layout::RIGHT, Layout::UP, Layout::DOWN };
    for (char end : ends) {
        if (layout.isOpenEnd(end)) {
            open += end;
        }
    }
    while (true) {
        std::string answer = View::readWord("Where should " + tile.toString()
                                            + " go? (" + open + "): ");
        if (answer.length() == 1) {
            char letter = static_cast<char>(std::toupper(answer[0]));
            if (open.find(letter) != std::string::npos) {
                return letter;
            }
        }
        View::message("  Please type one of: " + open);
    }
}
