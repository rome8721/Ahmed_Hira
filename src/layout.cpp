// layout.cpp — Ahmed Hira
// Implementation of the Layout class. See layout.h for the class layout.

#include <sstream>
#include "layout.h"

// Default constructor
// Pseudocode:
//   no tiles anywhere, no spinner
Layout::Layout() : m_line(), m_up(), m_down(), m_spinnerIndex(NO_SPINNER) {}

// Copy constructor
// Pseudocode:
//   copy the three tile lists and the spinner position
Layout::Layout(const Layout& other)
    : m_line(other.m_line), m_up(other.m_up), m_down(other.m_down),
      m_spinnerIndex(other.m_spinnerIndex) {}

// Copy assignment
// Pseudocode:
//   copy the three tile lists and the spinner position
Layout& Layout::operator=(const Layout& other) {
    m_line = other.m_line;
    m_up = other.m_up;
    m_down = other.m_down;
    m_spinnerIndex = other.m_spinnerIndex;
    return *this;
}

std::vector<Tile> Layout::getLine() const { return m_line; }
std::vector<Tile> Layout::getUpArm() const { return m_up; }
std::vector<Tile> Layout::getDownArm() const { return m_down; }

// isOpenEnd
// Pseudocode:
//   L and R are open once any tile is on the table
//   U and D are open only once there is a spinner
bool Layout::isOpenEnd(char end) const {
    if (end == LEFT || end == RIGHT) {
        return !isEmpty();
    }
    if (end == UP || end == DOWN) {
        return hasSpinner();
    }
    return false;
}

// getEndPips
// Pseudocode:
//   closed end -> NO_SPINNER
//   L -> left pip of the first line tile;  R -> right pip of the last
//   U/D -> right pip of the arm's last tile, or the spinner pip if the arm is empty
int Layout::getEndPips(char end) const {
    if (!isOpenEnd(end)) {
        return NO_SPINNER;
    }
    switch (end) {
        case LEFT:  return m_line.front().getLeftPips();
        case RIGHT: return m_line.back().getRightPips();
        case UP:    return m_up.empty() ? spinnerPips() : m_up.back().getRightPips();
        case DOWN:  return m_down.empty() ? spinnerPips() : m_down.back().getRightPips();
        default:    return NO_SPINNER;
    }
}

// getOpenEndSum
// Pseudocode:
//   empty table -> 0
//   one tile on the line -> both its pips (a double counts both too)
//   otherwise -> value at the L tile + value at the R tile
//   if there is a spinner:
//     for each arm: if it has tiles, add the value of its outer tile
//                   else add the spinner pip, but only when the spinner is
//                   not itself the L or R end (then it was already counted)
int Layout::getOpenEndSum() const {
    if (isEmpty()) {
        return 0;
    }
    int sum = 0;
    if (m_line.size() == 1) {
        sum = m_line.front().getSum();
    } else {
        sum += endValue(m_line.front(), m_line.front().getLeftPips());
        sum += endValue(m_line.back(), m_line.back().getRightPips());
    }
    if (hasSpinner()) {
        int lastIndex = static_cast<int>(m_line.size()) - 1;
        bool spinnerAtEdge = (m_spinnerIndex == 0 || m_spinnerIndex == lastIndex);
        if (!m_up.empty()) {
            sum += endValue(m_up.back(), m_up.back().getRightPips());
        } else if (!spinnerAtEdge) {
            sum += spinnerPips();
        }
        if (!m_down.empty()) {
            sum += endValue(m_down.back(), m_down.back().getRightPips());
        } else if (!spinnerAtEdge) {
            sum += spinnerPips();
        }
    }
    return sum;
}

// canPlace
// Pseudocode:
//   bad end letter -> false
//   empty table -> any tile may start the layout
//   closed end -> false
//   otherwise one pip of the tile must equal the pips at that end
bool Layout::canPlace(const Tile& tile, char end) const {
    if (!isEndLetter(end)) {
        return false;
    }
    if (isEmpty()) {
        return true;
    }
    if (!isOpenEnd(end)) {
        return false;
    }
    int pips = getEndPips(end);
    return tile.getLeftPips() == pips || tile.getRightPips() == pips;
}

// toString
// Pseudocode:
//   "L", then each line tile left to right, then "R"
//   the spinner is printed as "[D <down arm, outer to inner> spinner <up arm, inner to outer> U]"
std::string Layout::toString() const {
    std::string text = "L";
    for (size_t i = 0; i < m_line.size(); i++) {
        if (static_cast<int>(i) == m_spinnerIndex) {
            text += " [D";
            for (int k = static_cast<int>(m_down.size()) - 1; k >= 0; k--) {
                text += " " + m_down[k].flipped().toString();
            }
            text += " " + m_line[i].toString();
            for (const Tile& tile : m_up) {
                text += " " + tile.toString();
            }
            text += " U]";
        } else {
            text += " " + m_line[i].toString();
        }
    }
    text += " R";
    return text;
}

// place
// Pseudocode:
//   refuse illegal moves
//   empty table: the tile becomes the line; a double is the spinner
//   L: flip so the tile's RIGHT touches the line; insert at front; spinner index shifts by one
//   R: flip so the tile's LEFT touches the line; append
//   U/D: flip so the tile's LEFT touches the arm; append to that arm
//   a double placed on the line when there is no spinner yet becomes the spinner
bool Layout::place(Tile tile, char end) {
    if (!canPlace(tile, end)) {
        return false;
    }
    if (isEmpty()) {
        m_line.push_back(tile);
        if (tile.isDouble()) {
            m_spinnerIndex = 0;
        }
        return true;
    }
    int pips = getEndPips(end);
    switch (end) {
        case LEFT:
            if (tile.getRightPips() != pips) {
                tile = tile.flipped();
            }
            m_line.insert(m_line.begin(), tile);
            if (hasSpinner()) {
                m_spinnerIndex++;
            } else if (tile.isDouble()) {
                m_spinnerIndex = 0;
            }
            break;
        case RIGHT:
            if (tile.getLeftPips() != pips) {
                tile = tile.flipped();
            }
            m_line.push_back(tile);
            if (!hasSpinner() && tile.isDouble()) {
                m_spinnerIndex = static_cast<int>(m_line.size()) - 1;
            }
            break;
        case UP:
            if (tile.getLeftPips() != pips) {
                tile = tile.flipped();
            }
            m_up.push_back(tile);
            break;
        case DOWN:
            if (tile.getLeftPips() != pips) {
                tile = tile.flipped();
            }
            m_down.push_back(tile);
            break;
        default:
            return false;
    }
    return true;
}

// fromString
// Pseudocode:
//   split into words; strip a leading "L" and trailing "R"
//   walk the words: tiles before "[D" go to leftPart, tiles after "U]" go to rightPart,
//     tiles between go to group (the spinner and its arms as one chain)
//   no group -> the line is leftPart alone, no spinner
//   with a group -> orient it as a chain; the spinner is the double in the group whose
//     pips match the neighbouring line tiles (first double if there are no neighbours);
//     tiles before it are the down arm (reverse and flip them so left touches the spinner),
//     tiles after it are the up arm
//   orient the whole line; if anything fails, leave this layout unchanged
bool Layout::fromString(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> words;
    std::string word;
    while (in >> word) {
        words.push_back(word);
    }
    if (!words.empty() && words.front() == "L") {
        words.erase(words.begin());
    }
    if (!words.empty() && words.back() == "R") {
        words.pop_back();
    }

    std::vector<Tile> leftPart, group, rightPart;
    bool inGroup = false, groupSeen = false;
    for (const std::string& w : words) {
        if (w == "[D") {
            if (groupSeen) return false;
            inGroup = true;
            groupSeen = true;
            continue;
        }
        if (w == "U]") {
            if (!inGroup) return false;
            inGroup = false;
            continue;
        }
        Tile tile;
        if (!Tile::fromString(w, tile)) {
            return false;
        }
        if (inGroup) {
            group.push_back(tile);
        } else if (groupSeen) {
            rightPart.push_back(tile);
        } else {
            leftPart.push_back(tile);
        }
    }
    if (inGroup) {
        return false;
    }

    std::vector<Tile> newLine, newUp, newDown;
    int newSpinner = NO_SPINNER;

    if (!groupSeen) {
        newLine = leftPart;
    } else {
        if (group.empty() || !orientChain(group)) {
            return false;
        }
        // Pick the spinner inside the group.
        int spinnerInGroup = NO_SPINNER;
        for (size_t k = 0; k < group.size(); k++) {
            if (!group[k].isDouble()) continue;
            int pips = group[k].getLeftPips();
            bool leftOk = leftPart.empty()
                || leftPart.back().getLeftPips() == pips || leftPart.back().getRightPips() == pips;
            bool rightOk = rightPart.empty()
                || rightPart.front().getLeftPips() == pips || rightPart.front().getRightPips() == pips;
            if (leftOk && rightOk) {
                spinnerInGroup = static_cast<int>(k);
                break;
            }
        }
        if (spinnerInGroup == NO_SPINNER) {
            return false;
        }
        for (int k = spinnerInGroup - 1; k >= 0; k--) {
            newDown.push_back(group[k].flipped());
        }
        for (size_t k = spinnerInGroup + 1; k < group.size(); k++) {
            newUp.push_back(group[k]);
        }
        newLine = leftPart;
        newSpinner = static_cast<int>(newLine.size());
        newLine.push_back(group[spinnerInGroup]);
        newLine.insert(newLine.end(), rightPart.begin(), rightPart.end());
    }

    if (!newLine.empty() && !orientChain(newLine)) {
        return false;
    }
    m_line = newLine;
    m_up = newUp;
    m_down = newDown;
    m_spinnerIndex = newSpinner;
    return true;
}

// clear
// Pseudocode:
//   empty all three lists and forget the spinner
void Layout::clear() {
    m_line.clear();
    m_up.clear();
    m_down.clear();
    m_spinnerIndex = NO_SPINNER;
}

// endName
std::string Layout::endName(char end) {
    switch (end) {
        case LEFT:  return "Left";
        case RIGHT: return "Right";
        case UP:    return "Up";
        case DOWN:  return "Down";
        default:    return "?";
    }
}

// isEndLetter
bool Layout::isEndLetter(char end) {
    return end == LEFT || end == RIGHT || end == UP || end == DOWN;
}

// spinnerPips
// Pseudocode:
//   the spinner's left pip (it is a double, so either pip works); NO_SPINNER if none
int Layout::spinnerPips() const {
    if (!hasSpinner()) {
        return NO_SPINNER;
    }
    return m_line[m_spinnerIndex].getLeftPips();
}

// endValue
// Pseudocode:
//   a double at an open end shows both pips; any other tile shows one
int Layout::endValue(const Tile& tile, int exposedPips) {
    if (tile.isDouble()) {
        return tile.getSum();
    }
    return exposedPips;
}

// orientChain
// Pseudocode:
//   for the first tile, if its right does not touch tile 2 but its left does, flip it
//   for every later tile, flip it if needed so its left equals the previous right
//   fail if a tile cannot be made to touch
bool Layout::orientChain(std::vector<Tile>& tiles) {
    if (tiles.size() < 2) {
        return true;
    }
    Tile& first = tiles[0];
    const Tile& second = tiles[1];
    bool rightTouches = first.getRightPips() == second.getLeftPips()
                     || first.getRightPips() == second.getRightPips();
    bool leftTouches  = first.getLeftPips() == second.getLeftPips()
                     || first.getLeftPips() == second.getRightPips();
    if (!rightTouches && leftTouches) {
        first = first.flipped();
    }
    for (size_t i = 1; i < tiles.size(); i++) {
        int need = tiles[i - 1].getRightPips();
        if (tiles[i].getLeftPips() != need) {
            tiles[i] = tiles[i].flipped();
        }
        if (tiles[i].getLeftPips() != need) {
            return false;
        }
    }
    return true;
}
