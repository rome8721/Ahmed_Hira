# Secondary workflow: resume a saved game (the "database read")

Loading happens in **two passes**:

1. `Serializer::load` turns text into a `GameState` record. It checks the *format*: numbers are numbers, tiles are tiles, no duplicate in a hand, and the next player has a block.
2. `Round::restore` turns the record into model objects. It checks the *game*: the layout connects, the boneyard has no duplicates, and both players are present.

```mermaid
sequenceDiagram
    autonumber
    actor U as Player
    participant T as Tournament
    participant V as View
    participant S as Serializer
    participant Ti as Tile
    participant H as Hand
    participant R as Round
    participant L as Layout
    participant B as Boneyard
    participant P as Players

    T->>T: askResume(saved)
    T->>V: readYesNo("Resume a saved game?")
    U-->>V: y
    loop until load succeeds or the user gives up
        T->>V: readWord("Enter the save file name: ")
        U-->>V: test
        T->>S: Serializer::load("test", state)
        S->>S: std::ifstream, then for each line: trim()
        Note over S: "Name:" alone starts a player block. Otherwise match a prefix with startsWith()
        S->>S: "Tournament Score:" outside a block is the goal, inside a block the player total
        S->>H: parseHand("1-1 5-6 ...") calls Tile::fromString + Hand::addTile for each word
        S->>S: Layout / Boneyard / Next Player lines copied as text
        alt any bad line, number, tile, or missing next player
            S-->>T: false
            T->>V: message("  Could not read a saved game from 'test'.")
            T->>V: readYesNo("Try another file name?")
        else
            S-->>T: true
        end
    end
    Note over T: m_tournamentScore = saved.tournamentScore

    T->>R: Round round(&m_human, &m_computer, MAX_TARGET)
    T->>R: restore(saved)
    R->>L: layout.fromString("L 0-4 4-6 R")
    L->>L: split words, find [D ... U] group, orientChain(), pick spinner
    R->>Ti: Tile::fromString(word) for each boneyard word
    R->>B: boneyard.setTiles(tiles)  [false if duplicates]
    R->>R: both names present in state.players? nextPlayer is one of them?
    R->>P: setHand(ps.hand), setScores(roundScore, tournamentScore)
    R->>R: m_layout, m_boneyard, m_targetScore, m_next = values from file
    alt restore returned false
        T->>V: message("That save file does not describe a valid game. Starting fresh.")
        Note over T: continue: ask for a target and deal a new round. The goal from the file is kept
    else
        T->>V: message("Resuming the saved round. Target score is 5.")
        T->>R: play()
    end
```

**Things to notice**

- `restore()` builds a *local* `Layout` and `Boneyard` first, and copies them into the round only after every check passes. So a failed restore leaves the round unchanged. This pattern is called "validate, then commit".
- **Not checked on load:** that all 28 tiles appear exactly once overall (a tile can be in a hand *and* the boneyard), and that the target is 3 or 5 (`Target Score: 4` is accepted and used). Both were confirmed by running the program.
- An empty `Boneyard:` line trims to `"Boneyard:"`, which looks like a player block header ("ends in `:`, no spaces"). So the loader quietly adds a fake player named "Boneyard". It happens to be harmless because `restore()` ignores unknown names, but it's a trap if you change the loader.
