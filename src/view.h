// view.h — Ahmed Hira
// All screen output and keyboard input for the game lives here, so the
// model classes (Tile, Hand, Boneyard, Layout, ...) stay free of I/O.

#ifndef VIEW_H
#define VIEW_H

#include <string>
#include "hand.h"
#include "layout.h"

namespace View {
    // ----- output -----
    void message(const std::string& text);
    void blankLine();

    // ASCII picture of the table with L/R/U/D labelled and doubles drawn cross-wise.
    void showLayout(const Layout& layout);

    // One player's hand, e.g. "Human's hand: 0-1 2-5 4-4 (sum 16)".
    void showHand(const std::string& owner, const Hand& hand);

    // ----- input (all validated; re-asks until the answer is acceptable) -----
    int readInt(const std::string& prompt, int minValue, int maxValue);
    bool readYesNo(const std::string& prompt);
    std::string readWord(const std::string& prompt);
    std::string readLine(const std::string& prompt);
}

#endif
