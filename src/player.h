// player.h — Ahmed Hira
// Base class for the two players. Human and Computer inherit from it and
// override chooseMove(); everything else is shared. The round never needs
// to know which kind of player it is talking to.

#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "hand.h"
#include "layout.h"
#include "move.h"

class Player {
public:
    // ----- constructors / destructor -----
    Player(const std::string& name = "Player");
    virtual ~Player();

    // ----- selectors -----
    inline std::string getName() const { return m_name; }
    inline Hand getHand() const { return m_hand; }
    inline int getHandSum() const { return m_hand.getSum(); }
    inline bool hasTiles() const { return !m_hand.isEmpty(); }
    inline int getRoundScore() const { return m_roundScore; }
    inline int getTournamentScore() const { return m_tournamentScore; }

    // True if any tile in the hand can be placed somewhere on the layout.
    bool canPlay(const Layout& layout) const;

    // ----- mutators -----
    bool addTile(Tile tile);
    bool removeTile(Tile tile);
    bool setHand(const Hand& hand);
    void clearHand();

    // Add points earned this round. Returns false for a negative amount.
    bool addRoundPoints(int points);

    // Set both scores directly (used when resuming). Returns false if either is negative.
    bool setScores(int roundScore, int tournamentScore);

    // Fold the round score into the tournament score and reset it to zero.
    void finishRound();

    // ----- the polymorphic part -----
    // Decide which tile to place and where. The layout is guaranteed to
    // accept at least one tile from this player's hand when this is called.
    virtual Move chooseMove(const Layout& layout, int targetScore) = 0;

protected:
    std::string m_name;
    Hand m_hand;
    int m_roundScore;
    int m_tournamentScore;
};

#endif
