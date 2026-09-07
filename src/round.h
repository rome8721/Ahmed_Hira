// round.h — Ahmed Hira
// One round: deal, decide who starts, alternate turns until a hand is empty
// or neither player can move, then award the bonus. The two players are
// held as Player* so the round never knows (or cares) which is which.

#ifndef ROUND_H
#define ROUND_H

#include <string>
#include "player.h"
#include "layout.h"
#include "boneyard.h"
#include "gamestate.h"

class Round {
public:
    // ----- constants -----
    static const int PLAYER_COUNT = 2;
    static const int HAND_SIZE = 7;
    static const int MIN_TARGET = 3;
    static const int MAX_TARGET = 5;

    enum Outcome { FINISHED, SAVE_REQUESTED };

    // ----- constructors -----
    // firstDrawer draws first when deciding who starts (the human, per the rules).
    Round(Player* firstDrawer, Player* secondDrawer, int targetScore);

    // ----- selectors -----
    inline int getTargetScore() const { return m_targetScore; }
    inline Layout getLayout() const { return m_layout; }
    inline Boneyard getBoneyard() const { return m_boneyard; }
    std::string getNextPlayerName() const;

    // Copy the round into a save record (tournament score supplied by the caller).
    void toState(int tournamentScore, GameState& state) const;

    // ----- mutators -----
    // Fresh round: shuffle, deal, decide the first player.
    void startNew();

    // Continue a saved round. Returns false if the record does not fit these players.
    bool restore(const GameState& state);

    // Play until the round ends or the user asks to save.
    Outcome play();

private:
    Player* m_players[PLAYER_COUNT];
    int m_next;                 // index of the player about to move
    int m_targetScore;
    Layout m_layout;
    Boneyard m_boneyard;
    int m_consecutivePasses;

    void dealHands();
    void chooseFirstPlayer();

    // One player's turn: draw until able to play, then place. False = passed.
    bool takeTurn(Player* player);

    // Bonus for the lighter hand, then fold round scores into tournament scores.
    void finish();

    void showTable() const;
};

#endif
