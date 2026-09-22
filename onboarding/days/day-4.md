# Day 4: Data and integrations

**Goal:** Understand how game data is shaped (model objects vs plain records), how it's saved and loaded, what counts as "config" here, what touches the outside world, and follow one tile through its whole life.

**Time:** about 5 hours.

---

## First: what "integrations" means in this project (10 min)

This program has **no database, no network, no external services and no third-party libraries**. Its only contact with the outside world is:

| Boundary | Code | Direction |
|---|---|---|
| Terminal | `View` (`std::cout`, `std::getline(std::cin)`) | in and out |
| Save files | `Serializer` (`std::ofstream`, `std::ifstream`) | in and out |
| Randomness | `Boneyard::shuffle()` (`std::random_device` + `std::mt19937`) | in |

So today, "integration" means **the save file**, and "the data model" means **the classes and the `GameState` record**.

## What to read

| Order | File | Focus | Time |
|---|---|---|---|
| 1 | [diagrams/data-model.md](../diagrams/data-model.md) | Both ER diagrams, the save-file line table, and the invariants table | 30 min |
| 2 | [src/gamestate.h](../../src/gamestate.h) | The "DTO" records: `PlayerState`, `GameState` | 10 min |
| 3 | [src/round.cpp](../../src/round.cpp) `toState()` and `restore()` | Model → record → model | 25 min |
| 4 | [src/serializer.cpp](../../src/serializer.cpp) `save()`, `load()`, `parseHand()`, `startsWith()`, `trim()` | Record ↔ text | 30 min |
| 5 | [src/layout.cpp](../../src/layout.cpp) `toString()`, `fromString()`, `orientChain()` | The trickiest text format | 30 min |
| 6 | [src/boneyard.cpp](../../src/boneyard.cpp) `reset()`, `shuffle()`, `setTiles()`, `hasDuplicates()` | Where tiles are born | 10 min |
| 7 | [diagrams/save-game.md](../diagrams/save-game.md), [diagrams/resume-game.md](../diagrams/resume-game.md), [diagrams/new-round.md](../diagrams/new-round.md) | The three secondary flows that move data | 25 min |
| 8 | [Makefile](../../Makefile), [.vscode/tasks.json](../../.vscode/tasks.json), [.vscode/launch.json](../../.vscode/launch.json), [.vscode/settings.json](../../.vscode/settings.json) | Build and debug config | 15 min |

## Diagram / simulator for today

- [diagrams/data-model.md](../diagrams/data-model.md)
- Simulator scenario 1, the last two steps (`toState` → `Serializer::save`).

---

## Key idea: model objects vs records ("entities vs DTOs")

| | Model objects (`Hand`, `Layout`, `Boneyard`, `Player`) | Records (`GameState`, `PlayerState`) |
|---|---|---|
| Fields | `private`, reached through methods | `public`, plain struct fields |
| Rules | Enforce them: no duplicates, legal placements, no negative scores | None: they just carry values |
| Layout / boneyard stored as | Lists of oriented `Tile`s | **Text** (`layoutText`, `boneyardText`) |
| Who creates them | `Round`, `Tournament` | `Round::toState`, `Serializer::load` |
| Why | So the game can't get into an illegal state | So `Round` and `Serializer` never depend on each other |

The **conversion points** are where bugs like to hide: `Round::toState` (model → record), `Round::restore` (record → model), `Serializer::save`/`load` (record ↔ text). Day 5's first bug lives in one of these.

## Key idea: what counts as "config" here

There are no `.env` or settings files for the *game*. Its configuration is:

| Kind | Where | Values |
|---|---|---|
| Game rules (compile-time constants) | `Round::HAND_SIZE`, `PLAYER_COUNT`, `MIN_TARGET`, `MAX_TARGET` in [round.h](../../src/round.h) | 7, 2, 3, 5 |
| | `Tile::MIN_PIPS`, `MAX_PIPS` in [tile.h](../../src/tile.h) | 0, 6 |
| | `Boneyard::FULL_SET_SIZE` in [boneyard.h](../../src/boneyard.h) | 28 (documentation only, not used in logic) |
| | `Tournament::MIN/MAX_TOURNAMENT_SCORE` in [tournament.h](../../src/tournament.h) | 1, 10000 |
| Build | [Makefile](../../Makefile) `CXX`, `CXXFLAGS`, `SRC`, `HDR`, `TARGET` | `g++`, `-std=c++17 -Wall -Wextra -g`. The `ifeq` blocks pick `program.exe` on Windows, `program` elsewhere, and add a macOS-only `-isysroot` flag. Rebuilds when any `.cpp` or `.h` changes |
| IDE build | tasks in [tasks.json](../../.vscode/tasks.json) | **build (make)** just runs `make` (use this one). **build** is the original macOS task, with the compiler flags written out a second time |
| Debug | [launch.json](../../.vscode/launch.json) | *Windows, gdb* configuration: runs `${workspaceFolder}/program.exe` under `C:/msys64/ucrt64/bin/gdb.exe`, `cwd` = repo root, in a separate console window, pre-launch task **build (make)** |
| Runtime choices (typed by the user) | `Tournament::run` | tournament score, target 3/5, save-file names |

Changing a constant means **recompiling**. Also note that the Makefile and the macOS `build` task each list the compiler flags: change one and the other can silently drift. That's why the **build (make)** task calls `make` instead of repeating them.

**One rule that isn't in any config file:** the save-file format is **fixed by the course**. Code may read and write it, but must never change its layout.

---

## Hands-on exercises

### 1. Follow one tile from birth to disk and back (75 min)

Follow tile **5-6** through the whole program. For each step write the `file:function`, and what the tile looks like there (object or text, orientation). Use the debugger where it helps.

1. **Born:** `Boneyard::reset()`. Which loop iteration creates it? What orientation?
2. **Shuffled and dealt:** `Boneyard::shuffle()`, then `Round::dealHands()` → `drawTile` → `Player::addTile` → `Hand::addTile`.
3. **Chosen:** typed as `"5-6"` → `Tile::fromString` → `Hand::contains`.
4. **Placed:** `Layout::place` stores it as `6-5` on the Right (happy-path scenario).
5. **Saved:** `Round::toState` → `Layout::toString()` → `GameState::layoutText` → `Serializer::save` → the line `Layout:   L 0-4 4-6 6-5 R`.
6. **Loaded:** `Serializer::load` → `layoutText` → `Round::restore` → `Layout::fromString` → `orientChain`.

Tip: for step 1–2 start a *new* game (answer `n`) with a breakpoint in `Hand::addTile` and the condition `tile.m_leftPips == 5 && tile.m_rightPips == 6`.

Deliverable: a small diagram or table of the tile's journey. You'll reuse it on Day 6.

### 2. Break the save file on purpose (60 min)

Copy `test` to `onboarding/saves/broken.txt`. Make **one** change at a time, resume it, and record: (a) the message you see, (b) which function rejected it, and (c) whether it was rejected in `Serializer::load` (format) or `Round::restore` (game rules).

| # | Change | Your result |
|---|---|---|
| a | Human's hand: change `1-5` to `6-1` (now `1-6` appears twice) | |
| b | Layout: `L 0-4 5-6 R` (tiles don't connect) | |
| c | `Next Player: Bob` | |
| d | Boneyard: add a second `1-4` | |
| e | `Current Score: ten` | |
| f | Rename `Human:` to `Humans:` | |
| g | A tile `5-7` in a hand | |
| h | `Target Score: 4` | |
| i | Put `0-5` (already in the boneyard) into the human's hand too | |

### 3. Save → edit → resume round-trip (20 min)

Play a new game for a few turns and save it. Open the file, and **predict** each line's source (`Round::toState` field → model getter). Then resume it and check the table matches.

### 4. Config experiment (20 min)

In a scratch branch (`git switch -c day4-experiment`), change `Round::HAND_SIZE` from 7 to 3 and rebuild. Start a new game. What changes? Now try 14. What's printed about who goes first, and why? (Read `chooseFirstPlayer`.) Undo with `git restore src/round.h`, then `git switch -` back.

---

## Check questions

1. Name the two record types used for saving, and explain in one sentence why `Round` doesn't write the file itself.
2. The save file contains `Tournament Score:` three times. How does `Serializer::load` tell what each one means?
3. The layout is stored in `GameState` as text rather than as a list of `Tile`s. Where is that text produced and where is it parsed back?
4. Give two things a hand-edited save file can get wrong that the program will **not** detect.
5. What are *all* the ways this program talks to the world outside itself?

---

---

## Answer key

**Exercise 2 (verified by running):**
a) "Could not read a saved game…": `Serializer::parseHand` → `Hand::addTile` refuses the duplicate (format stage).
b) "That save file does not describe a valid game. Starting fresh.": `Layout::fromString` → `orientChain` fails (restore stage).
c) "Could not read…": `load` returns false because there's no player block named Bob.
d) "…does not describe a valid game…": `Boneyard::setTiles` → `hasDuplicates` (restore stage).
e) "Could not read…": the number parse fails in `load`.
f) "…does not describe a valid game…": `load` succeeds (a block "Humans" exists), but `restore` can't find a player named "Human".
g) "Could not read…": `Tile::fromString` → `setPips` rejects 7.
h) **Accepted.** The game resumes with target 4 (`load` only checks `> 0`).
i) **Accepted.** No check that each tile appears once across hands, layout and boneyard.

You'll close both gaps (h and i) yourself on Day 6.

**Exercise 4:** With 3, each player gets 3 tiles and the boneyard has 22. With 14, all 28 tiles are dealt, so the first-player draw can't draw. `chooseFirstPlayer` silently falls back to `m_next = 0` (the Human always starts) and prints nothing about who plays first.

**Check questions**

1. `PlayerState` and `GameState` ([gamestate.h](../../src/gamestate.h)). `Round` fills a record and `Serializer` writes it, so the save format lives in one place (`Serializer`), and game logic never needs to know about files (separation of concerns).
2. By context: `load` remembers the current `section`. A line that's just `Name:` opens a player block, and `Layout:`, `Boneyard:` and `Next Player:` close it. `Tournament Score:` outside a block is the session's goal (`state.tournamentScore`). Inside a block it's that player's running total (`players[section].tournamentScore`).
3. Produced by `Layout::toString()`, called from `Round::toState()`. Parsed by `Layout::fromString()`, called from `Round::restore()`, which uses `orientChain()` to re-orient tiles and find the spinner inside `[D … U]`.
4. Any two of: a target other than 3 or 5; the same tile in a hand *and* the boneyard (or missing tiles); a tournament goal of 0; an empty `Hand:` line (it would be taken as a new block named "Hand").
5. The terminal (`View`: `std::cout` / `std::cin`), save files (`Serializer`: `std::ofstream` / `std::ifstream`), and the system's random source (`std::random_device` in `Boneyard::shuffle`). There's no network, database or external service.
