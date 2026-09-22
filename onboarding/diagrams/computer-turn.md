# Secondary workflow: the computer's turn (and help mode)

`Round` treats both players the same way. The only difference is what happens inside `chooseMove()`. For the computer, that's `Computer::recommend()`, the program's only strategy. **Help mode** (the human types `help`) calls the same function, so there's one strategy, not two.

**Real example** (resume the repo's [`test`](../../test) file): table `L 0-4 4-6 R`, target 5, computer hand `1-1 5-6 1-2 6-6 3-4 3-3`. Output:

```
The computer placed 5-6 at the Right of the layout.
It did so to earn a score of 5.
Computer placed 5-6 at the Right. Open-end total is 5, a multiple of 5: 5 points!
```

```mermaid
sequenceDiagram
    autonumber
    participant R as Round
    participant C as Computer (a Player)
    participant L as Layout (real table)
    participant X as Layout trial (a copy)
    participant V as View

    R->>C: canPlay(m_layout)
    C-->>R: true (the draw loop is the same as for the human)
    R->>C: chooseMove(m_layout, 5)  [virtual, runs Computer::chooseMove]
    C->>C: recommend(m_hand, layout, 5, reason)  [static]

    Note over C,L: Step 0: list every legal (tile, end) pair
    loop each tile in hand x each end L/R/U/D
        C->>L: canPlace(tile, end)
        L-->>C: true or false
    end
    Note over C: legal = [5-6 at R, 6-6 at R]  (one entry per tile if the table is empty)

    alt no legal moves
        C-->>C: reason = "because it has no legal move", return Move() (valid=false)
    end

    Note over C,X: Step 1: find the highest-scoring move
    loop each legal move
        C->>C: pointsFor(layout, move, 5)
        C->>X: sumAfter: Layout trial(layout) then trial.place(move)
        X-->>C: trial.getOpenEndSum()
        Note over C: 5-6 at R gives 0+5 = 5 (multiple of 5, scores 5). 6-6 at R gives 0+12 = 12 (no)
    end
    Note over C: ties go to a double, then to the heavier tile

    alt bestPoints > 0
        C-->>C: reason = "to earn a score of 5", return best
    else nothing scores
        Note over C,X: Step 2: if any double is legal, play the heaviest double. Step 3: otherwise play the heaviest tile. For the same tile, pick the end that leaves the smallest open-end total (sumAfter again)
        C-->>C: reason = "because no move scores, ..." return pick
    end

    C->>V: message("The computer placed 5-6 at the Right of the layout.")
    C->>V: message("It did so to earn a score of 5.")
    C-->>R: Move{5-6, 'R', valid}
    Note over R: from here it's identical to the human flow: place, removeTile, getOpenEndSum, score, message
```

## Help mode (inside `Human::chooseMove`)

```mermaid
sequenceDiagram
    actor U as Player
    participant P as Human
    participant V as View
    participant C as Computer (static function only)

    P->>V: readWord("Enter the tile to play ...")
    U-->>V: help   (or h)
    V-->>P: "help"
    P->>C: Computer::recommend(m_hand, layout, targetScore, reason)
    C-->>P: Move suggestion + reason text
    alt suggestion.valid
        P->>V: message("  Suggestion: place 1-5 at the Right, because ...")
    else
        P->>V: message("  Suggestion: nothing in your hand can be placed.")
    end
    Note over P: continue: ask again. Help never plays a tile for you
```

**Things to notice**

- `sumAfter()` copies the `Layout` (`Layout trial(layout);`) and plays on the copy. That's why `Layout` has a hand-written copy constructor, and why the computer "thinking" never changes the real table.
- `recommend()` is `static`: it uses only its parameters, never a `Computer` object's data. That's what lets `Human` call it.
- In help mode, the reason text is written from the computer's point of view ("it played its heaviest tile…"), even though it's shown to the human.
