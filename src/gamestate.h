// gamestate.h — Ahmed Hira
// Everything needed to save a game and pick it up later. Plain records:
// the Serializer reads/writes them, the Round fills and consumes them.

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <map>
#include <string>
#include "hand.h"

struct PlayerState {
    Hand hand;
    int roundScore;
    int tournamentScore;

    PlayerState() : hand(), roundScore(0), tournamentScore(0) {}
};

struct GameState {
    int tournamentScore;                          // score that ends the tournament
    int targetScore;                              // 3 or 5 for this round
    std::map<std::string, PlayerState> players;   // keyed by player name
    std::string layoutText;                       // "L ... R"
    std::string boneyardText;                     // "0-6 1-5 ..."
    std::string nextPlayer;                       // name of the player to move

    GameState() : tournamentScore(0), targetScore(0), players(), layoutText(),
                  boneyardText(), nextPlayer() {}
};

#endif
