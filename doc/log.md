# Project Log — Ahmed Hira

## 2026-09-06 (Day 1) — ~0.5 hours
**What I did:**
- Created project folder structure: `Ahmed_Hira/src/` and `Ahmed_Hira/doc/`
- Started this log file
- Set up Makefile with `build`, `run`, and `clean` targets so I can rebuild in one keystroke (`make`)
- Added a stub `src/main.cpp` and verified it compiles and runs

**Bugs:** none yet

**Successes:** clean build environment working on day 1

---

## 2026-09-06 (Step 1: Tile) — ~0.5 hours
**What I did:**
- Wrote the `Tile` class (`src/tile.h`, `src/tile.cpp`): two pip values, `getSum()`, `isDouble()`, `flipped()`, `toString()`, and `==` that treats 2-5 and 5-2 as the same tile
- Replaced the stub `main.cpp` with a small test that prints a few tiles and checks each function
- Built with `make`, no warnings; output matched what I expected

- Got the course C++ guidelines and reworked `Tile` to match: `MIN_PIPS`/`MAX_PIPS` constants instead of magic numbers, constructor rejects out-of-range pips, `setPips()` mutator returns true/false, small selectors are inline, pseudocode comments on each function

**Bugs:** none

**Successes:** first real class working and matching the style guide; next is `Hand`

---

## 2026-09-06 (Step 2: Hand) — ~0.5 hours
**What I did:**
- Read the 35 Domino rules page and saved it with the course notes; confirmed the set is double-six (28 tiles), not 4-4
- Wrote the `Hand` class (`src/hand.h`, `src/hand.cpp`): a vector of `Tile`, with `addTile()` (rejects duplicates), `removeTile()` (false if absent), `getSum()`, `contains()`, `getTile()`, `toString()` in save-file form, and a copy constructor
- Test in `main.cpp` covers adds, a flipped duplicate, remove, out-of-range index, copy independence, and clear

**Bugs:** none

**Successes:** Hand works and its text form already matches the serialization format; next is `Boneyard`

---

## 2026-09-06 (Step 3: Boneyard) — ~0.5 hours
**What I did:**
- Wrote the `Boneyard` class (`src/boneyard.h`, `src/boneyard.cpp`): `reset()` builds the 28-tile double-six set with a two-loop pattern (right starts at left so each pair appears once), `shuffle()` uses `std::shuffle` with `std::mt19937`, `drawTile()` takes from the front and returns false when empty, `addToBottom()` for the first-player rule, `setTiles()` for resuming from a file (rejects duplicates)
- Test in `main.cpp` deals a 7-tile hand, does the first-player draw-and-return, drains the boneyard, and tries a bad resume list

**Bugs:** none

**Successes:** Tile, Hand, and Boneyard all work together; next is `Layout`

---

## 2026-09-06 (Steps 4-8: Layout, players, Round, Tournament, save/resume) — ~3 hours
**What I did:**
- `Layout` (`layout.h/.cpp`): main line L..R, spinner index, U and D arms. `place()` orients tiles so touching pips match; only the first double becomes the spinner. `getOpenEndSum()` counts a double at an end as both pips, and empty spinner arms count the spinner pip only once the spinner has tiles on both sides. `toString()`/`fromString()` use the save-file form `L 5-3 [D 3-3 U] 3-2 2-2 R`. Tested against all four plays from the rules page (5, 8, 13, 15).
- `Move` struct (`move.h`): tile + end letter + valid flag.
- `Player` base (`player.h/.cpp`) with protected hand and scores; `Human` and `Computer` override `chooseMove()`. `Round` only ever holds `Player*`, so it cannot tell them apart.
- `Computer::recommend()` is the strategy: best-scoring move first (ties: double, then heavier tile); otherwise dump the heaviest double; otherwise the heaviest tile, at the end that leaves the smallest open-end total. Prints tile, end, and reason. `Human` help mode calls the same function.
- `View` (`view.h/.cpp`): all keyboard/screen I/O so model classes stay clean. ASCII layout draws doubles cross-wise and marks the spinner.
- `Round` (`round.h/.cpp`): deal 7 each, first-player draw with tiles returned to the boneyard bottom, draw-until-playable or pass, scoring, end-of-round bonus `(heavier - lighter) div target`, save prompt after each placement.
- `Tournament` (`tournament.h/.cpp`): resume-or-new, tournament score, target 3 or 5 per round, winner/draw announcement.
- `Serializer` + `GameState` (`serializer.h/.cpp`, `gamestate.h`): read/write the course text format.
- Tests: resumed from the exact example file on the rules page, computer chose 2-4 at R for 15 as expected; scripted bad human inputs were all rejected; saved and resumed again successfully. Two-computer self-play for 40 rounds finished cleanly with no illegal moves.

**Bugs:**
- My Layout test placed 5-3 twice, so later placements were "wrongly" refused. Not a Layout bug: duplicate tiles are Hand's job. Fixed the test.
- Compiler warned that a class with a hand-written copy constructor should also declare copy assignment. Added `operator=` to Hand, Boneyard, Layout.

**Successes:** week 1 target met and most of week 2 (strategy, help, serialization, target 3 or 5) done. Remaining: play a real game by hand, read the rubric, and polish output.

---

<!-- Template for future entries:

## YYYY-MM-DD — X hours
**What I did:**
-

**Bugs:**
-

**Successes:**
-

---
-->
