# Request flow: one human turn

The single most important flow in the program is **one turn**: the player types a tile and an end, and the program checks it, places it, scores it, and reports back. The whole program is this flow run in a loop, with `Computer` in place of `Human` every other time.

In web terms: the "UI click" is the player typing `5-6` and `R`, the "controller" is `Round`, the "validation" is `Tile`, `Hand` and `Layout`, and the "database write" is changing the in-memory `Layout`, `Hand` and score. Writing to disk happens only if the player then chooses to save (see [save-game.md](save-game.md)).

**Sample data** comes from [`saves/happy-path.txt`](../saves/happy-path.txt): target 5, table `L 0-4 4-6 R` (open ends L=0, R=6), human hand `1-6 1-5 2-4 3-6 2-2 5-6`. The human types `5-6` then `R`. You can reproduce it exactly:

```sh
make && ./program      # then: y, onboarding/saves/happy-path.txt, 5-6, R
```

## Sequence diagram

```mermaid
sequenceDiagram
    autonumber
    actor U as Player (terminal)
    participant T as Tournament
    participant R as Round
    participant P as Human (a Player)
    participant V as View
    participant Ti as Tile
    participant H as Hand
    participant L as Layout
    participant B as Boneyard

    T->>R: play()
    Note over R: current = m_players[m_next] (the Human)
    R->>V: showLayout(m_layout), showHand(...) x2, message(...)
    V-->>U: prints table, both hands, boneyard, scores
    R->>R: takeTurn(current)

    rect rgba(128,128,128,0.12)
    Note over R,B: Draw loop. Skipped in the happy path because the human can already play
    loop while canPlay(m_layout) is false
        R->>P: canPlay(m_layout)
        P->>L: canPlace(tile, end) for every tile x L/R/U/D
        L-->>P: false for all
        P-->>R: false
        R->>B: drawTile(drawn)
        alt boneyard empty
            B-->>R: false
            R->>V: message("... passes.")
            R-->>R: return false (pass)
        else drew a tile
            B-->>R: true, drawn = front tile
            R->>P: addTile(drawn)
            R->>V: message("... draws X from the boneyard.")
        end
    end
    end

    R->>P: canPlay(m_layout)
    P->>L: canPlace(1-6, 'L') false, canPlace(1-6, 'R') true
    P-->>R: true

    R->>P: chooseMove(m_layout, 5)  [virtual call, runs Human::chooseMove]
    loop until the input is legal
        P->>V: readWord("Enter the tile to play ...")
        V-->>U: prompt
        U-->>V: types "5-6"
        V-->>P: "5-6"
        P->>Ti: Tile::fromString("5-6", tile)
        Ti-->>P: true, tile = 5-6
        P->>H: m_hand.contains(5-6)
        H-->>P: true (found at index 5)
        P->>P: askEnd(layout, 5-6)
        P->>L: isOpenEnd('L'/'R'/'U'/'D')
        L-->>P: open = "LR"
        P->>V: readWord("Where should 5-6 go? (LR): ")
        U-->>V: types "R"
        V-->>P: "R" then toupper gives 'R'
        P->>L: canPlace(5-6, 'R')
        L->>L: getEndPips('R') = 6, tile has a 6
        L-->>P: true
    end
    P-->>R: Move{tile 5-6, end 'R', valid true}

    R->>L: place(move)
    L->>L: canPlace again, then flip 5-6 to 6-5 so 6 touches 6, push_back
    L-->>R: true (line is now 0-4 4-6 6-5)
    R->>P: removeTile(5-6)
    P->>H: removeTile(5-6)
    H-->>P: true
    R->>L: getOpenEndSum()
    L-->>R: 5  (L end 0 + R end 5)
    Note over R: points = (5 mod 5 == 0) ? 5 : 0, so 5
    R->>P: addRoundPoints(5)
    R->>V: message("Human placed 5-6 at the Right. Open-end total is 5, a multiple of 5: 5 points!")
    V-->>U: prints result
    R-->>R: takeTurn returns true

    Note over R: back in play(): passes = 0, hand not empty, m_next = 1 (Computer)
    R->>V: readYesNo("Save the game and quit?")
    V-->>U: prompt
    alt player answers y
        R-->>T: return SAVE_REQUESTED, see save-game.md
    else player answers n
        R->>R: loop again, now the Computer's turn (see computer-turn.md)
    end
```

## The same flow as a numbered checklist (with file:line)

Line numbers are from the current code and may drift a little.

| # | Where | What happens | Data at this point |
|---|---|---|---|
| 1 | [tournament.cpp:53](../../src/tournament.cpp#L53) `round.play()` | Tournament hands control to the round | `m_targetScore = 5`, `m_next = 0` (Human) |
| 2 | [round.cpp:112–115](../../src/round.cpp#L112-L115) `Round::play` | Pick the current player, print the table, call `takeTurn` | `current` = `&m_human` (seen through a `Player*`) |
| 3 | [round.cpp:182](../../src/round.cpp#L182) → [player.cpp:16](../../src/player.cpp#L16) `Player::canPlay` | Can any tile go anywhere? If not, draw ([boneyard.cpp:99](../../src/boneyard.cpp#L99)) or pass | `true`: `1-6` fits on R |
| 4 | [round.cpp:193](../../src/round.cpp#L193) `player->chooseMove(...)` | Virtual call. C++ runs `Human::chooseMove` because the object is a `Human` | `layout = L 0-4 4-6 R`, `targetScore = 5` |
| 5 | [human.cpp:21](../../src/human.cpp#L21) → [view.cpp:128](../../src/view.cpp#L128) `View::readWord` | Print prompt, read a line, return its first word | `answer = "5-6"` |
| 6 | [human.cpp:35](../../src/human.cpp#L35) → [tile.cpp:63](../../src/tile.cpp#L63) `Tile::fromString` | Must be exactly `digit-digit`, each 0–6 | `tile = {left 5, right 6}` |
| 7 | [human.cpp:39](../../src/human.cpp#L39) → [hand.cpp:58](../../src/hand.cpp#L58) `Hand::contains` | Is it in the hand? (`5-6` and `6-5` count as the same tile) | `indexOf = 5`, so `true` |
| 8 | [human.cpp:43](../../src/human.cpp#L43) → [human.cpp:58](../../src/human.cpp#L58) `Human::askEnd` | List open ends, read a letter, upper-case it | `open = "LR"`, `end = 'R'` |
| 9 | [human.cpp:44](../../src/human.cpp#L44) → [layout.cpp:109](../../src/layout.cpp#L109) `Layout::canPlace` | Does a pip on the tile equal the pips at that end? | `getEndPips('R') = 6`, so `true` |
| 10 | [human.cpp:50](../../src/human.cpp#L50) `return Move(tile, end)` | Package the decision | `Move{5-6, 'R', valid=true}` |
| 11 | [round.cpp:194](../../src/round.cpp#L194) → [layout.cpp:156](../../src/layout.cpp#L156) `Layout::place` | Re-check, flip if needed, append | stored as `6-5`, line = `0-4 4-6 6-5` |
| 12 | [round.cpp:198](../../src/round.cpp#L198) → [hand.cpp:93](../../src/hand.cpp#L93) `Hand::removeTile` | Take it out of the hand | hand = `1-6 1-5 2-4 3-6 2-2` |
| 13 | [round.cpp:199](../../src/round.cpp#L199) → [layout.cpp:75](../../src/layout.cpp#L75) `getOpenEndSum` | Add up open ends | `0 + 5 = 5` |
| 14 | [round.cpp:200–201](../../src/round.cpp#L200-L201) → [player.cpp:45](../../src/player.cpp#L45) | Score if it's a multiple of the target | `5 % 5 == 0`, so `m_roundScore 0 → 5` |
| 15 | [round.cpp:209](../../src/round.cpp#L209) `View::message` | Tell the player | "…Open-end total is 5, a multiple of 5: 5 points!" |
| 16 | [round.cpp:116–134](../../src/round.cpp#L116-L134) | Reset passes, check for empty hand, switch player, offer save | `m_next = 1` (Computer) |

## Validation: who rejects what

| Bad input | Rejected by | Message |
|---|---|---|
| `5_6`, `abc`, `56` | `Tile::fromString` (wrong shape) | "That is not a tile. Type two pip counts joined by a dash, like 2-5." |
| `9-9` | `Tile::fromString` → `Tile::setPips` (out of range 0–6) | same as above |
| `0-0` when you don't hold it | `Hand::contains` | "0-0 is not in your hand." |
| `2-4` on the L end showing 0 | `Layout::canPlace` | "2-4 does not match the 0 at the Left end." |
| `Q` as an end | `Human::askEnd` (not in the open-ends list) | "Please type one of: LR" |
| An illegal move that got past the Human | `Layout::place` returns false in `Round::takeTurn` | "…offered an illegal move and forfeits the turn." (safety net: shouldn't happen) |
