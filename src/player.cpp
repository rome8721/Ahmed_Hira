// player.cpp — Ahmed Hira
// Implementation of the shared Player behaviour. See player.h.

#include "player.h"

Player::Player(const std::string& name)
    : m_name(name), m_hand(), m_roundScore(0), m_tournamentScore(0) {}

Player::~Player() {}

// canPlay
// Pseudocode:
//   for each tile in hand, for each of the four ends:
//     if the layout accepts the tile there, return true
//   return false
bool Player::canPlay(const Layout& layout) const {
    const char ends[] = { Layout::LEFT, Layout::RIGHT, Layout::UP, Layout::DOWN };
    for (const Tile& tile : m_hand.getTiles()) {
        for (char end : ends) {
            if (layout.canPlace(tile, end)) {
                return true;
            }
        }
    }
    return false;
}

bool Player::addTile(Tile tile) {
    return m_hand.addTile(tile);
}

bool Player::removeTile(Tile tile) {
    return m_hand.removeTile(tile);
}

bool Player::setHand(const Hand& hand) {
    m_hand = hand;
    return true;
}

void Player::clearHand() {
    m_hand.clear();
}

bool Player::addRoundPoints(int points) {
    if (points < 0) {
        return false;
    }
    m_roundScore += points;
    return true;
}

bool Player::setScores(int roundScore, int tournamentScore) {
    if (roundScore < 0 || tournamentScore < 0) {
        return false;
    }
    m_roundScore = roundScore;
    m_tournamentScore = tournamentScore;
    return true;
}

// finishRound
// Pseudocode:
//   tournament total += this round's points; round points back to zero
void Player::finishRound() {
    m_tournamentScore += m_roundScore;
    m_roundScore = 0;
}
