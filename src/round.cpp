// round.cpp — Ahmed Hira
// Implementation of the Round class. See round.h.

#include <sstream>
#include "round.h"
#include "view.h"

Round::Round(Player* firstDrawer, Player* secondDrawer, int targetScore)
    : m_next(0), m_targetScore(targetScore), m_layout(), m_boneyard(), m_consecutivePasses(0) {
    m_players[0] = firstDrawer;
    m_players[1] = secondDrawer;
}

std::string Round::getNextPlayerName() const {
    return m_players[m_next]->getName();
}

// toState
// Pseudocode:
//   copy target, layout text, boneyard text, next player's name,
//   and each player's hand and scores keyed by name
void Round::toState(int tournamentScore, GameState& state) const {
    state = GameState();
    state.tournamentScore = tournamentScore;
    state.targetScore = m_targetScore;
    state.layoutText = m_layout.toString();
    state.boneyardText = m_boneyard.toString();
    state.nextPlayer = getNextPlayerName();
    for (Player* player : m_players) {
        PlayerState ps;
        ps.hand = player->getHand();
        ps.roundScore = player->getRoundScore();
        ps.tournamentScore = player->getTournamentScore();
        state.players[player->getName()] = ps;
    }
}

// startNew
// Pseudocode:
//   full shuffled boneyard, empty table, deal seven each, pick who starts
void Round::startNew() {
    m_boneyard.reset();
    m_boneyard.shuffle();
    m_layout.clear();
    m_consecutivePasses = 0;
    for (Player* player : m_players) {
        player->clearHand();
    }
    dealHands();
    chooseFirstPlayer();
}

// restore
// Pseudocode:
//   every player must appear in the record; the next player must be one of them
//   rebuild layout and boneyard from their text; give each player its hand and scores
bool Round::restore(const GameState& state) {
    Layout layout;
    if (!layout.fromString(state.layoutText)) {
        return false;
    }
    Boneyard boneyard;
    Hand asList;
    std::vector<Tile> tiles;
    {
        std::istringstream in(state.boneyardText);
        std::string word;
        while (in >> word) {
            Tile tile;
            if (!Tile::fromString(word, tile)) return false;
            tiles.push_back(tile);
        }
    }
    if (!boneyard.setTiles(tiles)) {
        return false;
    }
    int nextIndex = -1;
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (state.players.count(m_players[i]->getName()) == 0) {
            return false;
        }
        if (m_players[i]->getName() == state.nextPlayer) {
            nextIndex = i;
        }
    }
    if (nextIndex < 0) {
        return false;
    }
    for (Player* player : m_players) {
        const PlayerState& ps = state.players.at(player->getName());
        player->setHand(ps.hand);
        player->setScores(ps.roundScore, ps.tournamentScore);
    }
    m_layout = layout;
    m_boneyard = boneyard;
    m_targetScore = state.targetScore;
    m_next = nextIndex;
    m_consecutivePasses = 0;
    return true;
}

// play
// Pseudocode:
//   loop:
//     show the table; current player takes a turn
//     placed a tile -> passes reset; if their hand is now empty the round is over
//     passed -> count it; two passes in a row means nobody can move, round over
//     switch players
//     after a placement, offer to save and quit
Round::Outcome Round::play() {
    while (true) {
        Player* current = m_players[m_next];
        showTable();
        View::message("--- " + current->getName() + "'s turn (target " + std::to_string(m_targetScore) + ") ---");
        bool placed = takeTurn(current);
        if (placed) {
            m_consecutivePasses = 0;
            if (!current->hasTiles()) {
                View::message(current->getName() + " has played every tile.");
                finish();
                return FINISHED;
            }
        } else {
            m_consecutivePasses++;
            if (m_consecutivePasses >= PLAYER_COUNT) {
                View::message("Neither player can place a tile.");
                finish();
                return FINISHED;
            }
        }
        m_next = (m_next + 1) % PLAYER_COUNT;
        if (placed && View::readYesNo("Save the game and quit?")) {
            return SAVE_REQUESTED;
        }
    }
}

// dealHands
// Pseudocode:
//   for each player, draw HAND_SIZE tiles from the boneyard into their hand
void Round::dealHands() {
    for (Player* player : m_players) {
        for (int i = 0; i < HAND_SIZE; i++) {
            Tile drawn;
            if (m_boneyard.drawTile(drawn)) {
                player->addTile(drawn);
            }
        }
    }
}

// chooseFirstPlayer
// Pseudocode:
//   repeat: player 0 draws a tile, player 1 draws the next; both go back on the bottom
//   the higher pip total starts; on a tie draw again
void Round::chooseFirstPlayer() {
    while (true) {
        Tile first, second;
        if (!m_boneyard.drawTile(first) || !m_boneyard.drawTile(second)) {
            m_next = 0;   // cannot happen with 14 tiles left, but stay safe
            return;
        }
        View::message(m_players[0]->getName() + " draws " + first.toString() + ", "
                      + m_players[1]->getName() + " draws " + second.toString() + ".");
        m_boneyard.addToBottom(first);
        m_boneyard.addToBottom(second);
        if (first.getSum() != second.getSum()) {
            m_next = (first.getSum() > second.getSum()) ? 0 : 1;
            View::message(m_players[m_next]->getName() + " plays first.");
            return;
        }
        View::message("Same pips; drawing again.");
    }
}

// takeTurn
// Pseudocode:
//   while the player cannot play: draw; if the boneyard is empty, pass
//   ask the player for a move (Human asks the keyboard, Computer uses its strategy)
//   place it, remove it from the hand, score it if the open-end sum is a multiple of target
bool Round::takeTurn(Player* player) {
    while (!player->canPlay(m_layout)) {
        Tile drawn;
        if (!m_boneyard.drawTile(drawn)) {
            View::message(player->getName() + " cannot play and the boneyard is empty. "
                          + player->getName() + " passes.");
            return false;
        }
        player->addTile(drawn);
        View::message(player->getName() + " cannot play and draws " + drawn.toString()
                      + " from the boneyard.");
    }
    Move move = player->chooseMove(m_layout, m_targetScore);
    if (!m_layout.place(move)) {
        View::message(player->getName() + " offered an illegal move and forfeits the turn.");
        return false;
    }
    player->removeTile(move.tile);
    int sum = m_layout.getOpenEndSum();
    int points = (sum % m_targetScore == 0) ? sum : 0;
    player->addRoundPoints(points);
    std::string note = player->getName() + " placed " + move.tile.toString() + " at the "
                     + Layout::endName(move.end) + ". Open-end total is " + std::to_string(sum);
    if (points > 0) {
        note += ", a multiple of " + std::to_string(m_targetScore) + ": " + std::to_string(points) + " points!";
    } else {
        note += ": no points.";
    }
    View::message(note);
    return true;
}

// finish
// Pseudocode:
//   compare the two hand sums; the lighter hand earns (heavier - lighter) / target
//   print round scores and running tournament totals; fold round into tournament
void Round::finish() {
    Player* a = m_players[0];
    Player* b = m_players[1];
    View::blankLine();
    View::message("=== Round over ===");
    for (Player* player : m_players) {
        View::showHand(player->getName(), player->getHand());
    }
    if (a->getHandSum() != b->getHandSum()) {
        Player* lighter = (a->getHandSum() < b->getHandSum()) ? a : b;
        Player* heavier = (lighter == a) ? b : a;
        int bonus = (heavier->getHandSum() - lighter->getHandSum()) / m_targetScore;
        lighter->addRoundPoints(bonus);
        View::message(lighter->getName() + " has the lighter hand and earns a bonus of "
                      + std::to_string(bonus) + " (" + std::to_string(heavier->getHandSum()) + " - "
                      + std::to_string(lighter->getHandSum()) + ") div " + std::to_string(m_targetScore) + ".");
    } else {
        View::message("Both hands weigh the same; no bonus.");
    }
    for (Player* player : m_players) {
        View::message(player->getName() + ": " + std::to_string(player->getRoundScore())
                      + " this round, " + std::to_string(player->getTournamentScore() + player->getRoundScore())
                      + " in the tournament so far.");
        player->finishRound();
    }
}

void Round::showTable() const {
    View::blankLine();
    View::showLayout(m_layout);
    for (Player* player : m_players) {
        View::showHand(player->getName(), player->getHand());
    }
    View::message("Boneyard: " + std::to_string(m_boneyard.size()) + " tiles ("
                  + m_boneyard.toString() + ")");
    for (Player* player : m_players) {
        View::message(player->getName() + " score: " + std::to_string(player->getRoundScore())
                      + " this round, " + std::to_string(player->getTournamentScore()) + " before it.");
    }
}
