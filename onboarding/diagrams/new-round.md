# Secondary workflow: starting a new game and a new round

What happens from launching `./program` (and answering "no" to resume) until the first turn.

```mermaid
sequenceDiagram
    autonumber
    actor U as Player
    participant M as main()
    participant T as Tournament
    participant V as View
    participant R as Round
    participant B as Boneyard
    participant H as Human
    participant C as Computer

    M->>T: Tournament tournament (constructs m_human, m_computer)
    M->>T: run()
    T->>V: message("Welcome to 35 Domino!")
    T->>T: askResume(saved)
    T->>V: readYesNo("Resume a saved game?")
    U-->>V: n
    T->>V: readInt("Enter the tournament score to play to: ", 1, 10000)
    U-->>V: 7
    Note over T: m_tournamentScore = 7

    loop until someone reaches 7 (see round-end.md)
        T->>R: Round round(&m_human, &m_computer, MAX_TARGET)  [placeholder target]
        T->>V: readInt("Enter the target score for this round (3 or 5): ", 3, 5)
        U-->>V: 4
        T->>V: message("  The target must be exactly 3 or 5.")  [4 is in range 3..5 but not allowed]
        U-->>V: 5
        T->>R: round = Round(&m_human, &m_computer, 5)
        T->>R: startNew()
        R->>B: reset()  [28 tiles, 0-0 ... 6-6]
        R->>B: shuffle()  [std::mt19937 seeded by std::random_device]
        R->>R: m_layout.clear(), m_consecutivePasses = 0
        R->>H: clearHand()
        R->>C: clearHand()
        R->>R: dealHands()
        loop 7 times for Human, then 7 times for Computer
            R->>B: drawTile(drawn)
            R->>H: addTile(drawn)  (then C)
        end
        Note over B: 14 tiles left
        R->>R: chooseFirstPlayer()
        loop until the sums differ
            R->>B: drawTile(first)  [for m_players[0], the Human]
            R->>B: drawTile(second)  [for m_players[1], the Computer]
            R->>V: message("Human draws 4-4, Computer draws 2-5.")
            R->>B: addToBottom(first), addToBottom(second)
        end
        Note over R: m_next = index of the higher sum
        R->>V: message("Human plays first.")
        T->>R: play()  (see request-flow.md)
    end
```

**Things to notice**

- The Round is built twice: once with a placeholder target (`MAX_TARGET`), then reassigned with `round = Round(...)` after the player picks one. That works because `Layout` and `Boneyard` define copy assignment.
- `readInt(…, 3, 5)` accepts 4, so a second loop in `Tournament::run` rejects it.
- The first-player draw puts both tiles **back** on the bottom of the boneyard, so the boneyard still has 14 tiles.
- `chooseFirstPlayer` has a fallback (`m_next = 0`) for "boneyard ran out". The comment says it can't happen with 14 tiles, and that's true as long as `HAND_SIZE` stays 7.
