# Day 3: Trace it live

**Goal:** Watch the real program run the main flow in the debugger. Stop at every layer from the sequence diagram, inspect the data, and step through all three simulator scenarios plus the "pass" path.

**Time:** about 5 hours.

---

## What to read

| File | Why | Time |
|---|---|---|
| [ONBOARDING.md §4.4](../ONBOARDING.md#44-debug-in-vs-code-step-by-step) | Debugger controls refresher | 10 min |
| [.vscode/launch.json](../../.vscode/launch.json) (the *Windows, gdb* configuration) and the **build (make)** task in [.vscode/tasks.json](../../.vscode/tasks.json) | What F5 actually does: run `make` (which compiles with `-g`), then start `program.exe` under the `gdb` debugger in a separate console window | 15 min |
| [diagrams/request-flow.md](../diagrams/request-flow.md) | Your map for placing breakpoints | 10 min |
| [diagrams/round-end.md](../diagrams/round-end.md) | For the pass/round-end exercise | 15 min |

## Diagram / simulator for today

Keep [simulator.html](../simulator.html) open next to VS Code. For each scenario, advance the simulator one step each time the debugger stops, and check that the real values match.

---

## Setup: the breakpoint set (20 min)

Set these breakpoints (click left of the line number). Line numbers are approximate, so look for the code shown.

| # | Layer | File | Put the breakpoint on | Watch these |
|---|---|---|---|---|
| B1 | Control | [tournament.cpp](../../src/tournament.cpp) | `if (round.play() == Round::SAVE_REQUESTED)` (≈53) | `m_tournamentScore` |
| B2 | Control | [round.cpp](../../src/round.cpp) | `Player* current = m_players[m_next];` in `play()` (≈112) | `m_next`, `m_consecutivePasses` |
| B3 | Control | [round.cpp](../../src/round.cpp) | `while (!player->canPlay(m_layout))` in `takeTurn()` (≈182) | `m_boneyard` (expand `m_tiles`) |
| B4 | Control → Players | [round.cpp](../../src/round.cpp) | `Move move = player->chooseMove(...)` (≈193) | `player` (see its real type) |
| B5 | Players | [human.cpp](../../src/human.cpp) | `if (!Tile::fromString(answer, tile))` (≈35) | `answer` |
| B6 | Model | [hand.cpp](../../src/hand.cpp) | first line inside `Hand::indexOf` (≈114) | `tile`, `m_tiles` |
| B7 | Model | [layout.cpp](../../src/layout.cpp) | `int pips = getEndPips(end);` in `canPlace` (≈119) | `tile`, `end`, `pips` |
| B8 | Model | [layout.cpp](../../src/layout.cpp) | `int pips = getEndPips(end);` in `place` (≈167) | `tile`, `m_line` |
| B9 | Model | [boneyard.cpp](../../src/boneyard.cpp) | `if (isEmpty())` in `drawTile` (≈100) | `m_tiles` |
| B10 | Control | [round.cpp](../../src/round.cpp) | `int points = (sum % m_targetScore == 0) ? sum : 0;` (≈200) | `sum`, `m_targetScore` |

> **Tip:** B6 and B7 are hit *many* times, because `canPlay` and `recommend` call them in loops. Right-click a breakpoint → **Disable** to silence it, or **Edit Breakpoint** → add a condition such as `end == 'R'`.

---

## Hands-on exercises

### 1. Happy path in the debugger (60 min)

Select **Debug program (Windows, gdb)**, press F5. A console window opens: type `y`, then `onboarding/saves/happy-path.txt` there.

> Remember: **all typing goes into the separate console window**, and all inspecting happens in VS Code. Arrange the two windows side by side.

At each stop, write down the values in the **Watch these** column. Then:

- At **B4** press **F11** (Step Into). Which function do you land in? Look at the **Call Stack**.
- Continue to **B5**. Type `5-6` in the console window first (the program is waiting in `View::readLine`).
- At **B8**, press **F10** a few times and watch `tile` change from `5-6` to `6-5` when `flipped()` runs.
- At **B10**, what are `sum` and `points`?
- Answer `n` to "Save the game and quit?". Now the computer's turn starts. At **B4** press F11 again: this time you land in `Computer::chooseMove`. Same line, different function. That's polymorphism, live.

### 2. Draw path (40 min)

Restart (`Ctrl+Shift+F5`) and resume `onboarding/saves/draw-path.txt`. Disable B6/B7.

- At **B3**, the human can't play. Step Over (F10) and watch the loop run twice.
- At **B9**, check `m_tiles` before and after the draw. Which tile comes out first? Does it match the simulator?
- Find where the drawn tile is added to the hand (`player->addTile(drawn)`).

### 3. Validation path (40 min)

Resume `happy-path.txt` again and type the bad inputs from simulator scenario 3: `5_6`, `0-0`, `2-4` + `L`, `5-6` + `Q` + `r`.

For each bad input, find the exact line that decides "no", and the `continue` that sends it back to the top of the loop. Use **Step Over**, not Continue.

### 4. The pass path and the end of a round (40 min)

Resume `onboarding/saves/empty-boneyard.txt` (the human can't play and the boneyard is empty).

- At **B9**, `isEmpty()` is true. Step until you're back in `Round::play()`. What is `m_consecutivePasses`?
- Add a breakpoint at the start of `Round::finish()`. Continue playing (answer `n` to save prompts, play whatever you can, type `help` if stuck) until the round ends. At `finish()`, compute the bonus by hand from both hand sums before stepping over it.

### 5. Call Stack drill (15 min)

Stop anywhere inside `Layout::canPlace` during a **human** turn, and again during a **computer** turn. Copy both call stacks. Why are they different? (Hint: `Player::canPlay`, `Human::chooseMove`, `Computer::recommend`.)

---

## Check questions

1. You're paused at B4 and press F11. During the human's turn you land in `Human::chooseMove`, and during the computer's turn in `Computer::chooseMove`. Why does the same line lead to different functions?
2. The debugger seems "stuck" and no breakpoint is hit, and the terminal shows `Enter the tile to play ...`. What's going on?
3. The Makefile compiles with `-g` and without any `-O` optimisation flag. Why do both matter for debugging?
4. During the happy path, what's the value of `tile` just before and just after `tile = tile.flipped();` in `Layout::place`?
5. In the empty-boneyard scenario, trace the values of `m_consecutivePasses` and explain what would make the round end.

---

---

## Answer key

1. `player` is a `Player*`, but `chooseMove` is **virtual**, so the call is dispatched at run time on the object's real type. `m_players[0]` points to `Tournament::m_human` and `m_players[1]` to `Tournament::m_computer`.
2. The program is waiting for input in `View::readLine` → `std::getline(std::cin, …)`. Switch to the program's **console window** (it may be behind VS Code) and type your answer.
3. `-g` adds debug information (the map from machine code back to source lines and variable names). Without it, breakpoints don't bind and variables can't be shown. No optimisation (the default, same as `-O0`) keeps the machine code in the same order as your source. With optimisation (`-O2`), lines get reordered or merged and variables show as "optimized out", so stepping jumps around confusingly.
4. Before: `m_leftPips = 5, m_rightPips = 6` (5-6). After: `m_leftPips = 6, m_rightPips = 5` (6-5).
5. The human passes (`takeTurn` returns false), so `m_consecutivePasses` becomes **1**. If the computer then places a tile, it goes back to **0**. If the computer also can't play, it reaches **2** (`PLAYER_COUNT`) and the round ends with "Neither player can place a tile." In this file the computer holds 20 tiles and can play, so the count resets to 0. The round then continues until someone goes out, or both players are blocked.
