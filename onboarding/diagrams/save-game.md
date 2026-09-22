# Secondary workflow: save and quit (the "database write")

After every successful placement, `Round::play()` asks *"Save the game and quit?"*. On `y`, the whole round is copied into a `GameState` record and written to a text file.

```mermaid
sequenceDiagram
    autonumber
    actor U as Player
    participant R as Round
    participant T as Tournament
    participant GS as GameState (record)
    participant L as Layout
    participant B as Boneyard
    participant P as Players (Human, Computer)
    participant S as Serializer
    participant F as File on disk
    participant V as View

    Note over R: a tile was just placed and m_next already points at the NEXT player
    R->>V: readYesNo("Save the game and quit?")
    U-->>V: y
    R-->>T: return SAVE_REQUESTED
    T->>T: saveAndQuit(round)
    T->>R: round.toState(m_tournamentScore, state)
    R->>GS: state = GameState()  [start clean]
    R->>GS: tournamentScore, targetScore
    R->>L: toString()
    L-->>R: "L 0-4 4-6 6-5 R"
    R->>B: toString()
    B-->>R: "1-4 0-5 0-2 ..."
    R->>R: getNextPlayerName() gives "Computer"
    loop each player
        R->>P: getHand(), getRoundScore(), getTournamentScore()
        R->>GS: players[name] = PlayerState{hand, roundScore, tournamentScore}
    end
    loop until the write succeeds
        T->>V: readWord("Enter a file name to save to: ")
        U-->>V: mygame.txt
        T->>S: Serializer::save(state, "mygame.txt")
        S->>F: std::ofstream out(fileName)
        alt file cannot be opened
            S-->>T: false
            T->>V: message("  Could not write to 'mygame.txt'. Please try again.")
        else
            S->>F: Tournament Score, Target Score, Computer block, Human block, Layout, Boneyard, Next Player
            S-->>T: out.good()
        end
    end
    T->>V: message("Game saved to 'mygame.txt'. Goodbye!")
    Note over T: run() returns, main() returns 0, program exits
```

**Real output.** Resume [`saves/happy-path.txt`](../saves/happy-path.txt), play `5-6` `R`, then save:

```
Tournament Score: 7

Target Score: 5

Computer:
   Hand: 1-1 0-6 1-2 6-6 3-4 3-3
   Current Score: 10
   Tournament Score: 0

Human:
   Hand: 1-6 1-5 2-4 3-6 2-2
   Current Score: 5
   Tournament Score: 0

Layout:   L 0-4 4-6 6-5 R

Boneyard:  1-4 0-5 0-2 0-0 4-4 5-5 0-1 0-3 4-5 2-6 2-3 1-3 2-5 3-5

Next Player: Computer
```

Notice the human's `5-6` shows up in the layout as **`6-5`**, and `Current Score` went from 0 to 5.

**Things to notice**

- `Serializer::save` always writes **Computer first, then Human** (a hard-coded list of names, to match the course example). If you renamed a player, their block would silently disappear from the file.
- The save prompt appears only after a *placement*. `m_next` has already moved on, so "Next Player" is the player who hasn't moved yet. That's correct.
- Existing files are overwritten without asking.
