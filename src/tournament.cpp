// tournament.cpp — Ahmed Hira
// Implementation of the Tournament class. See tournament.h.

#include "tournament.h"
#include "round.h"
#include "serializer.h"
#include "view.h"

Tournament::Tournament() : m_human(), m_computer(), m_tournamentScore(0) {}

// run
// Pseudocode:
//   offer to resume; if resumed, restore that round and continue it
//   otherwise ask for the tournament score
//   repeat: (new round: ask for the target 3 or 5, deal) play the round
//           save requested -> write the file and stop
//           round finished -> print totals; stop when someone reaches the goal
void Tournament::run() {
    View::message("Welcome to 35 Domino!");
    GameState saved;
    bool resumed = askResume(saved);
    if (resumed) {
        m_tournamentScore = saved.tournamentScore;
    } else {
        m_tournamentScore = View::readInt("Enter the tournament score to play to: ",
                                          MIN_TOURNAMENT_SCORE, MAX_TOURNAMENT_SCORE);
    }

    while (true) {
        Round round(&m_human, &m_computer, Round::MAX_TARGET);
        if (resumed) {
            if (!round.restore(saved)) {
                View::message("That save file does not describe a valid game. Starting fresh.");
                resumed = false;
                continue;
            }
            View::message("Resuming the saved round. Target score is "
                          + std::to_string(round.getTargetScore()) + ".");
            resumed = false;
        } else {
            View::blankLine();
            int target = View::readInt("Enter the target score for this round (3 or 5): ",
                                       Round::MIN_TARGET, Round::MAX_TARGET);
            while (target != Round::MIN_TARGET && target != Round::MAX_TARGET) {
                View::message("  The target must be exactly 3 or 5.");
                target = View::readInt("Enter the target score for this round (3 or 5): ",
                                       Round::MIN_TARGET, Round::MAX_TARGET);
            }
            round = Round(&m_human, &m_computer, target);
            round.startNew();
        }

        if (round.play() == Round::SAVE_REQUESTED) {
            saveAndQuit(round);
            return;
        }

        View::blankLine();
        View::message("Tournament totals (playing to " + std::to_string(m_tournamentScore) + "):  Human "
                      + std::to_string(m_human.getTournamentScore()) + "  Computer "
                      + std::to_string(m_computer.getTournamentScore()));
        if (isOver()) {
            announceWinner();
            return;
        }
    }
}

// askResume
// Pseudocode:
//   ask yes/no; on yes ask for a file name and load it; retry or give up on failure
bool Tournament::askResume(GameState& state) {
    if (!View::readYesNo("Resume a saved game?")) {
        return false;
    }
    while (true) {
        std::string fileName = View::readWord("Enter the save file name: ");
        if (!fileName.empty() && Serializer::load(fileName, state)) {
            return true;
        }
        View::message("  Could not read a saved game from '" + fileName + "'.");
        if (!View::readYesNo("Try another file name?")) {
            return false;
        }
    }
}

// saveAndQuit
// Pseudocode:
//   copy the round into a record; ask for a file name until the write works
void Tournament::saveAndQuit(const Round& round) {
    GameState state;
    round.toState(m_tournamentScore, state);
    while (true) {
        std::string fileName = View::readWord("Enter a file name to save to: ");
        if (!fileName.empty() && Serializer::save(state, fileName)) {
            View::message("Game saved to '" + fileName + "'. Goodbye!");
            return;
        }
        View::message("  Could not write to '" + fileName + "'. Please try again.");
    }
}

bool Tournament::isOver() const {
    return m_human.getTournamentScore() >= m_tournamentScore
        || m_computer.getTournamentScore() >= m_tournamentScore;
}

// announceWinner
// Pseudocode:
//   higher tournament total wins; equal totals are a draw
void Tournament::announceWinner() const {
    View::blankLine();
    View::message("=== Tournament over ===");
    View::message("Human: " + std::to_string(m_human.getTournamentScore())
                  + "   Computer: " + std::to_string(m_computer.getTournamentScore()));
    if (m_human.getTournamentScore() > m_computer.getTournamentScore()) {
        View::message("The Human wins the tournament!");
    } else if (m_computer.getTournamentScore() > m_human.getTournamentScore()) {
        View::message("The Computer wins the tournament!");
    } else {
        View::message("The tournament is a draw.");
    }
}
