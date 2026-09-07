// serializer.cpp — Ahmed Hira
// Implementation of the Serializer. See serializer.h.

#include <fstream>
#include <sstream>
#include "serializer.h"

// save
// Pseudocode:
//   open the file; write each section in the format from the project page
bool Serializer::save(const GameState& state, const std::string& fileName) {
    std::ofstream out(fileName);
    if (!out) {
        return false;
    }
    out << "Tournament Score: " << state.tournamentScore << "\n\n";
    out << "Target Score: " << state.targetScore << "\n\n";
    // The course example lists Computer first, then Human.
    const char* order[] = { "Computer", "Human" };
    for (const char* name : order) {
        std::map<std::string, PlayerState>::const_iterator it = state.players.find(name);
        if (it == state.players.end()) {
            continue;
        }
        out << name << ":\n";
        out << "   Hand: " << it->second.hand.toString() << "\n";
        out << "   Current Score: " << it->second.roundScore << "\n";
        out << "   Tournament Score: " << it->second.tournamentScore << "\n\n";
    }
    out << "Layout:   " << state.layoutText << "\n\n";
    out << "Boneyard:  " << state.boneyardText << "\n\n";
    out << "Next Player: " << state.nextPlayer << "\n";
    return out.good();
}

// load
// Pseudocode:
//   read line by line, remembering which player section we are inside
//   a line that is just "Name:" starts a player section
//   "Tournament Score:" outside a section is the tournament target; inside, the player's total
//   "Hand:", "Current Score:", "Layout:", "Boneyard:", "Next Player:" fill the matching field
//   fail on unreadable numbers, bad tiles, or a missing next player
bool Serializer::load(const std::string& fileName, GameState& state) {
    std::ifstream in(fileName);
    if (!in) {
        return false;
    }
    state = GameState();
    std::string section;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        std::string rest;
        if (line.back() == ':' && line.find(' ') == std::string::npos) {
            section = line.substr(0, line.length() - 1);
            state.players[section] = PlayerState();
        } else if (startsWith(line, "Tournament Score:", rest)) {
            std::istringstream num(rest);
            int value;
            if (!(num >> value) || value < 0) return false;
            if (section.empty()) {
                state.tournamentScore = value;
            } else {
                state.players[section].tournamentScore = value;
            }
        } else if (startsWith(line, "Target Score:", rest)) {
            std::istringstream num(rest);
            if (!(num >> state.targetScore) || state.targetScore <= 0) return false;
        } else if (startsWith(line, "Hand:", rest)) {
            if (section.empty() || !parseHand(rest, state.players[section].hand)) return false;
        } else if (startsWith(line, "Current Score:", rest)) {
            std::istringstream num(rest);
            int value;
            if (section.empty() || !(num >> value) || value < 0) return false;
            state.players[section].roundScore = value;
        } else if (startsWith(line, "Layout:", rest)) {
            state.layoutText = rest;
            section.clear();
        } else if (startsWith(line, "Boneyard:", rest)) {
            state.boneyardText = rest;
            section.clear();
        } else if (startsWith(line, "Next Player:", rest)) {
            state.nextPlayer = rest;
            section.clear();
        } else {
            return false;   // a line we do not understand
        }
    }
    return !state.nextPlayer.empty() && state.players.count(state.nextPlayer) > 0;
}

// parseHand
// Pseudocode:
//   for each word: parse as a tile and add to the hand; fail on a bad tile or a duplicate
bool Serializer::parseHand(const std::string& text, Hand& hand) {
    hand.clear();
    std::istringstream in(text);
    std::string word;
    while (in >> word) {
        Tile tile;
        if (!Tile::fromString(word, tile) || !hand.addTile(tile)) {
            return false;
        }
    }
    return true;
}

std::string Serializer::trim(const std::string& text) {
    const char* SPACES = " \t\r\n";
    size_t start = text.find_first_not_of(SPACES);
    if (start == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(SPACES);
    return text.substr(start, end - start + 1);
}

bool Serializer::startsWith(const std::string& line, const std::string& prefix, std::string& rest) {
    if (line.compare(0, prefix.length(), prefix) != 0) {
        return false;
    }
    rest = trim(line.substr(prefix.length()));
    return true;
}
