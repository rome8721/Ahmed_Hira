// tournament.h — Ahmed Hira
// The whole session: optionally resume from a file, then play rounds until
// one player reaches the tournament score.

#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "human.h"
#include "computer.h"
#include "gamestate.h"

class Tournament {
public:
    // ----- constants -----
    static const int MIN_TOURNAMENT_SCORE = 1;
    static const int MAX_TOURNAMENT_SCORE = 10000;

    Tournament();

    // Run the entire program flow. Returns when the tournament ends or a game is saved.
    void run();

private:
    Human m_human;
    Computer m_computer;
    int m_tournamentScore;

    // Ask about resuming; on success fill state and return true.
    bool askResume(GameState& state);

    // Ask for a file name and save the round. Repeats until the write succeeds.
    void saveAndQuit(const class Round& round);

    // True once somebody has reached the tournament score.
    bool isOver() const;

    void announceWinner() const;
};

#endif
