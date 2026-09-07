// view.cpp — Ahmed Hira
// Implementation of the View functions. See view.h.

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "view.h"

namespace {
    // Width of one tile cell in the ASCII picture ("5-3" is 3 wide).
    const int CELL_WIDTH = 3;

    // Lower-case copy of a string.
    std::string lower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return text;
    }

    // Text of an arm from the spinner outward, or "(none)".
    std::string armText(const std::vector<Tile>& arm) {
        if (arm.empty()) {
            return "(none)";
        }
        std::string text;
        for (const Tile& tile : arm) {
            text += tile.toString() + " ";
        }
        return text;
    }
}

void View::message(const std::string& text) {
    std::cout << text << "\n";
}

void View::blankLine() {
    std::cout << "\n";
}

// showLayout
// Pseudocode:
//   empty table -> say so
//   build three rows for the main line: a normal tile sits in the middle row,
//     a double is drawn vertically (pip / | / pip) across the three rows,
//     and the spinner gets an asterisk so it stands out
//   print the U arm above, the rows, the D arm below, then the open-end values
void View::showLayout(const Layout& layout) {
    if (layout.isEmpty()) {
        message("Layout: (empty)  L R");
        return;
    }
    std::vector<Tile> line = layout.getLine();
    std::string top, middle, bottom;
    for (size_t i = 0; i < line.size(); i++) {
        const Tile& tile = line[i];
        std::string cell(CELL_WIDTH, ' ');
        if (tile.isDouble()) {
            std::string pip = std::to_string(tile.getLeftPips());
            bool spinner = (static_cast<int>(i) == layout.getSpinnerIndex());
            top    += " " + pip + " " + " ";
            middle += (spinner ? "*|*" : " | ") + std::string(" ");
            bottom += " " + pip + " " + " ";
        } else {
            top    += cell + " ";
            middle += tile.toString() + " ";
            bottom += cell + " ";
        }
    }
    if (layout.hasSpinner()) {
        message("   U (up from spinner *):   " + armText(layout.getUpArm()));
    }
    message("   " + top);
    message("L  " + middle + " R");
    message("   " + bottom);
    if (layout.hasSpinner()) {
        message("   D (down from spinner *): " + armText(layout.getDownArm()));
    }
    std::string ends = "Open ends:";
    const char letters[] = { Layout::LEFT, Layout::RIGHT, Layout::UP, Layout::DOWN };
    for (char end : letters) {
        if (layout.isOpenEnd(end)) {
            ends += std::string(" ") + end + "=" + std::to_string(layout.getEndPips(end));
        }
    }
    ends += "   total pips at open ends = " + std::to_string(layout.getOpenEndSum());
    message(ends);
}

void View::showHand(const std::string& owner, const Hand& hand) {
    message(owner + "'s hand: " + hand.toString() + "   (sum " + std::to_string(hand.getSum()) + ")");
}

// readInt
// Pseudocode:
//   repeat: print prompt, read a line, try to parse a whole integer in range
//   complain and loop on anything else
int View::readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::string text = readLine(prompt);
        std::istringstream in(text);
        int value;
        char extra;
        if (in >> value && !(in >> extra) && value >= minValue && value <= maxValue) {
            return value;
        }
        message("  Please enter a whole number from " + std::to_string(minValue)
                + " to " + std::to_string(maxValue) + ".");
    }
}

// readYesNo
// Pseudocode:
//   repeat until the answer starts with y or n
bool View::readYesNo(const std::string& prompt) {
    while (true) {
        std::string answer = lower(readWord(prompt + " (y/n): "));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no")  return false;
        message("  Please answer y or n.");
    }
}

// readWord
// Pseudocode:
//   read a line and return its first word (empty string if the line was blank)
std::string View::readWord(const std::string& prompt) {
    std::istringstream in(readLine(prompt));
    std::string word;
    in >> word;
    return word;
}

// readLine
// Pseudocode:
//   print the prompt and read one whole line; on end-of-input, quit cleanly
std::string View::readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        message("\nInput closed. Goodbye.");
        std::exit(0);
    }
    return line;
}
