# Secondary workflow: passing, ending a round, and ending the tournament

A round ends in one of two ways:

1. **A player goes out:** they place their last tile.
2. **Blocked:** two passes in a row (`m_consecutivePasses >= PLAYER_COUNT`). A pass happens when a player can't play **and** the boneyard is empty.

Try the blocked path with [`saves/empty-boneyard.txt`](../saves/empty-boneyard.txt): the human can't play and the boneyard is empty, so the human passes straight away.

```mermaid
sequenceDiagram
    autonumber
    participant T as Tournament
    participant R as Round
    participant P as current Player
    participant B as Boneyard
    participant V as View

    loop Round::play()
        R->>R: takeTurn(current)
        R->>P: canPlay(m_layout)
        P-->>R: false
        R->>B: drawTile(drawn)
        B-->>R: false (empty)
        R->>V: message("Human cannot play and the boneyard is empty. Human passes.")
        R-->>R: takeTurn returns false
        R->>R: m_consecutivePasses++  (now 1)
        Note over R: no save prompt after a pass. m_next switches to the other player
        alt other player also passes
            R->>R: m_consecutivePasses = 2
            R->>V: message("Neither player can place a tile.")
            R->>R: finish()
        else other player places a tile
            R->>R: m_consecutivePasses = 0
            alt their hand is now empty
                R->>V: message("Computer has played every tile.")
                R->>R: finish()
            end
        end
    end

    Note over R,V: Round::finish()
    R->>V: showHand(name, hand) for both players
    alt hand sums differ
        Note over R: lighter gets (heavier - lighter) / target, integer division
        R->>P: addRoundPoints(bonus) on the lighter player
        R->>V: message("Human has the lighter hand and earns a bonus of 2 (30 - 20) div 5.")
    else equal
        R->>V: message("Both hands weigh the same#59; no bonus.")
    end
    loop each player
        R->>V: message("Human: 7 this round, 7 in the tournament so far.")
        R->>P: finishRound()  [tournament += round, round = 0]
    end
    R-->>T: return FINISHED

    T->>V: message("Tournament totals (playing to 7): Human 7 Computer 15")
    T->>T: isOver()  [anyone's tournament score >= goal?]
    alt yes
        T->>V: announceWinner()  higher total wins, equal is a draw
        Note over T: run() returns, main() returns 0
    else no
        Note over T: loop back: new Round, ask for a new target (see new-round.md)
    end
```

**Things to notice**

- The bonus uses **integer division**: `(30 - 20) / 5 = 2`, but `(33 - 20) / 5 = 2` as well, not 2.6.
- `finishRound()` moves round points into the tournament total **and** resets the round score. The message line adds the two together *before* calling it, so the printed total is right.
- Ending the tournament uses `>=`. Both players can pass the goal in the same round. Then the higher total wins, or it's a draw.
