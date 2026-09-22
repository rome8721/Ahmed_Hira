# Day 2: The main flow

**Goal:** Explain, using real class and method names, everything that happens during one turn: from the player typing `5-6` to the points appearing on screen. Then read that code in order, layer by layer.

**Time:** about 5½–6 hours (simulator 75 min, C++ primer Part 2 60 min, code reading 2¼ h, exercises 80 min). The `Computer` material is optional if you run out of time.

---

## What to use first: the simulator (75 min)

Open [simulator.html](../simulator.html) in your browser (double-click it, no internet needed).

1. **Scenario 1, Happy path.** Step through it slowly with the → key. At every step, read the **Call** box and the **Data** box *before* the explanation, and try to predict what the data will be.
2. **Scenario 2, Draw from boneyard.** Notice where the flow is *different* from scenario 1, and where it's the *same*.
3. **Scenario 3, Validation errors.** For each red "rejected" step, note which class said no.
4. Answer the "Check yourself" question at the end of each scenario before revealing the answer.

Keep [diagrams/request-flow.md](../diagrams/request-flow.md) open in another tab. It's the same flow as one sequence diagram.

## Before the code: C++ primer, Part 2 (60 min)

Read [cpp-primer.md](../cpp-primer.md) **sections 8–13**. The code you read next uses references (`&`), pointers (`->`) and `virtual` on almost every line. Section 11 (inheritance and `virtual`) is the most important one today.

## What to read (in this exact order)

Read the **pseudocode comment** above each function first, then the code. Line numbers may drift slightly.

| # | Layer | File and function(s) | Time |
|---|---|---|---|
| 1 | Control | [round.cpp](../../src/round.cpp) `Round::play()` (≈ line 110) | 15 min |
| 2 | Control | [round.cpp](../../src/round.cpp) `Round::takeTurn()` (≈ line 181) | 15 min |
| 3 | Players | [player.cpp](../../src/player.cpp) `Player::canPlay()`, and [player.h](../../src/player.h) (`chooseMove` is `= 0`) | 10 min |
| 4 | Players | [human.cpp](../../src/human.cpp) `Human::chooseMove()`, `Human::askEnd()` | 20 min |
| 5 | I/O | [view.cpp](../../src/view.cpp) `View::readWord()`, `View::readLine()`, `View::message()` | 10 min |
| 6 | Model | [tile.cpp](../../src/tile.cpp) `Tile::fromString()`, `setPips()`, `operator==`, `flipped()` | 15 min |
| 7 | Model | [hand.cpp](../../src/hand.cpp) `contains()`, `indexOf()`, `removeTile()` | 10 min |
| 8 | Model | [layout.cpp](../../src/layout.cpp) `isOpenEnd()`, `getEndPips()`, `canPlace()`, `place()`, `getOpenEndSum()`, `endValue()` | 40 min |
| 9 | Players | *(Optional, or do it on Day 3)* [computer.cpp](../../src/computer.cpp) `Computer::chooseMove()`, `recommend()`, `pointsFor()`, `sumAfter()`, with [diagrams/computer-turn.md](../diagrams/computer-turn.md) | 30 min |

`layout.cpp` is the hardest file in the project. Read the big comment at the top of [layout.h](../../src/layout.h) first: it explains how tiles are stored ("oriented so that touching pips match").

---

## Hands-on exercises

### 1. Write the turn story (30 min)

Without looking, write 10–15 numbered lines describing one human turn. Each line should name a real `Class::method`. Then check it against the table in [request-flow.md](../diagrams/request-flow.md#the-same-flow-as-a-numbered-checklist-with-fileline).

### 2. Compute open-end totals by hand (20 min)

Use `getOpenEndSum()` rules (a double at an end counts both pips, and an empty spinner arm counts the spinner's pip only when the spinner is **not** at the L or R end). Work out the total for each layout (save-file notation):

| # | Layout | Your answer |
|---|---|---|
| a | `L 5-3 R` | |
| b | `L 5-3 3-3 R` | |
| c | `L 5-3 [D 3-3 U] 3-2 R` | |
| d | `L 5-3 [D 3-3 U] 3-2 2-2 R` | |
| e | `L 0-4 [D 4-4 U] R` | |

For each, would it score with target 5? With target 3?

### 3. Predict, then run (30 min)

For each question, write your prediction **first**, then check it in the program (`./program`, resume `onboarding/saves/happy-path.txt`):

1. You type `6-5` instead of `5-6`. Accepted? How is it stored on the table?
2. You type `help`. Which tile does it suggest, and why? Which function produced that sentence?
3. You type `5-6` and then `r` (lower-case). Accepted?
4. You type `1-6` and choose `L`. What message appears, and which class decided?

### 4. *(Optional)* Trace the computer's decision (20 min)

Resume the repo's `test` file. The computer plays `5-6` at the Right "to earn a score of 5". Using `Computer::recommend()`, list every legal `(tile, end)` pair the computer considered, the open-end total each would produce, and explain why `5-6` won over `6-6`.

---

## Check questions

1. Name, in order, the three classes that can reject a human's input during `Human::chooseMove()`, and one example input each rejects.
2. `Round::takeTurn` calls `player->chooseMove(...)`. How does C++ decide whether to run `Human::chooseMove` or `Computer::chooseMove`? What keyword in `player.h` makes this possible?
3. The player types `5-6` and places it on the Right end, which shows 6. How is the tile stored in `m_line`, and which function flips it?
4. `Layout::place()` calls `canPlace()` again, even though `Human` already checked. Why is that a good idea?
5. When does a placement score points, and how many?

---

---

## Answer key

**Exercise 2:** a = **8** (5 + 3). b = **11** (5 + 3-3 at the end counts 6). c = **13** (5 + 2 + empty up arm 3 + empty down arm 3). d = **15** (5 + 2-2 counts 4 + 3 + 3). e = **8** (0 + 4-4 counts 8. The spinner is at the R end, so its empty arms add nothing). With target 5: d scores 15. With target 3: d scores 15. All others score nothing with either target (8, 11, 13 aren't multiples of 3 or 5). All values were verified by running `Layout::fromString` + `getOpenEndSum`.

**Exercise 3:** (1) Accepted: `Tile::operator==` treats 6-5 and 5-6 as the same tile. It's stored as `6-5` either way. (2) `help` calls `Computer::recommend()`. The suggestion depends on the position: from `happy-path.txt` it suggests `5-6` at the Right "to earn a score of 5". (3) Yes: `askEnd` uses `toupper`. (4) "1-6 does not match the 0 at the Left end.", decided by `Layout::canPlace` (the message is printed by `View::message`).

**Exercise 4:** The legal pairs are `5-6` at R (total 0 + 5 = 5) and `6-6` at R (total 0 + 12 = 12, because a double at an end counts both pips). Nothing matches the L end (0). Only 5 is a multiple of 5, so `pointsFor` gives 5 vs 0, and `5-6` wins in step 1 of the strategy.

**Check questions**

1. `Tile::fromString` rejects badly formed text (`5_6`, `abc`, `9-9`). `Hand::contains` rejects a tile you don't hold (`0-0`). `Layout::canPlace` rejects a tile that doesn't match the chosen end (`2-4` on an end showing 0). (`Human::askEnd` also rejects an end letter that isn't open, like `Q`.)
2. **Dynamic dispatch** (polymorphism): `chooseMove` is declared `virtual ... = 0` (pure virtual) in `Player`. At run time C++ looks at the real type of the object the `Player*` points to. `Human` and `Computer` mark their versions with `override`.
3. As `6-5`. `Layout::place()`, in `case RIGHT`: if the tile's left pip doesn't equal the end's pips, it calls `tile.flipped()`. Stored tiles always satisfy `m_line[i].right == m_line[i+1].left`.
4. It's defence in depth: the model protects its own rules, whoever calls it. `Computer`, future code, or a bug in `Human` can't put an illegal tile on the table. If it happens anyway, `Round::takeTurn` forfeits the turn instead of corrupting the layout.
5. After a tile is placed, `Round::takeTurn` computes `m_layout.getOpenEndSum()`. If that total is a multiple of the round's target (3 or 5), the player scores **the whole total** (for example 15 points, not 1). Otherwise 0.
