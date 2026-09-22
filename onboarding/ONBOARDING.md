# Onboarding: 35 Domino (C++)

Welcome! This folder teaches you this codebase in 6 days. By the end you should be able to:

1. explain what the program does and how a turn flows through the code,
2. trace one turn through every class in a debugger, and
3. make a small change on your own.

**How to use this folder**

| Item | What it is |
|---|---|
| `ONBOARDING.md` (this file) | The overview: what the app does, vocabulary, where things live, how to build/run/debug |
| [`cpp-primer.md`](cpp-primer.md) | How basic programming ideas are written in C++, using examples from this code. Part 1 on Day 1, Part 2 on Day 2 |
| [`diagrams/`](diagrams/) | Mermaid diagrams. They render on GitHub, and in VS Code with a Mermaid preview extension |
| [`simulator.html`](simulator.html) | Open it in any browser. Step through one turn, class by class, with sample data |
| [`days/`](days/) | The 6-day plan. Do one file per day |
| [`saves/`](saves/) | Save files that start the program at an exact moment in a game (used by the simulator scenarios and the debugger exercises) |

---

## 1. What the app does (from a player's point of view)

It's a **dominoes game you play in the terminal against the computer**. The game is called *35 Domino*. There are no windows or buttons: the program prints text, and you type answers.

A session goes like this:

1. You start the program. It asks: *"Resume a saved game?"*
   - **No:** it asks what total to play to (the *tournament score*, for example 7).
   - **Yes:** it asks for a save-file name and continues that game from where it stopped.
2. Each **round** starts by asking for a **target score**: 3 or 5.
3. The 28 dominoes are shuffled. You and the computer each get 7. The rest go face-down in a pile called the **boneyard**.
4. You each draw one tile to decide who goes first. More pips (dots) goes first.
5. On your turn you type a tile you hold (like `5-6`) and which end of the table to put it on (`L`, `R`, and later `U`/`D`). Its number has to match the number on that end.
   - If you can't play anything, the game automatically draws from the boneyard for you until you can. If the boneyard is empty, you **pass**.
   - Type `help` and the computer suggests a move and tells you why.
6. **Scoring:** after every placement, add up the pips on all open ends of the table. If that total is a multiple of the target (3 or 5), the player who just played scores that total.
7. The round ends when someone plays their last tile, or when both players pass in a row. Whoever holds fewer pips gets a **bonus**: (heavier hand − lighter hand) ÷ target, rounded down.
8. Round scores are added to tournament totals. When someone reaches the tournament score, the program names the winner (or a draw).
9. After any placement you can choose **save and quit**. The game is written to a plain text file you can resume later.

The computer plays itself using a simple strategy (take the biggest score, otherwise dump heavy tiles). It prints the reason for every move.

> **Note on terminology.** Onboarding templates usually talk about "requests", "databases", "DTOs" and "external services". This project is a single console program, so those words map onto it like this:
>
> | Web/app term | What it means here |
> |---|---|
> | UI / front end | The terminal. All printing and reading happens in `View` ([src/view.cpp](../src/view.cpp)) |
> | A "request" | One turn: the player types a tile and an end, and the program places it and scores it |
> | Controller | `Tournament` (whole session) and `Round` (one round, the referee) |
> | Domain model / entities | `Tile`, `Hand`, `Boneyard`, `Layout`, `Player` |
> | DTO (data transfer object) | `GameState` and `PlayerState` ([src/gamestate.h](../src/gamestate.h)): plain records that carry data between `Round` and the save file |
> | Database | A **text save file** on disk, read and written by `Serializer` |
> | Config | The `Makefile`, `.vscode/*.json`, and `static const` numbers in the code (hand size, targets, pip range) |
> | External services | **None.** No network, no database server, no third-party libraries. Only the C++ standard library, the keyboard, the screen, and files |
> | Deployable | One executable file named `program` |

---

## 2. Glossary

### Game (domain) terms

| Term | Meaning | Where in code |
|---|---|---|
| **Tile / domino** | One piece with two ends. Each end shows 0–6 dots. Written `2-5` | `Tile` |
| **Pip** | One dot on a tile. "5 pips" means the end shows 5 | `m_leftPips`, `m_rightPips` |
| **Double** | A tile whose two ends match, like `3-3` | `Tile::isDouble()` |
| **Double-six set** | The full set of 28 tiles from `0-0` to `6-6`. Each combination appears once | `Boneyard::reset()`, `FULL_SET_SIZE` |
| **Hand** | The tiles one player holds | `Hand` |
| **Boneyard** | The face-down pile of undealt tiles. Draws come from the front, returned tiles go to the back | `Boneyard` |
| **Layout** | The tiles on the table | `Layout` |
| **Main line** | The row of tiles running left to right | `Layout::m_line` |
| **End / open end** | A place where a tile may be added. `L` left, `R` right, `U` up, `D` down | `Layout::LEFT/RIGHT/UP/DOWN`, `isOpenEnd()` |
| **Spinner** | The first double played. It opens two extra ends, Up and Down | `m_spinnerIndex`, `hasSpinner()` |
| **Arm** | The tiles growing out of the spinner, up or down | `m_up`, `m_down` |
| **Open-end total (open-end sum)** | Sum of the pips showing at all open ends. A double at an end counts **both** its pips | `Layout::getOpenEndSum()` |
| **Target score** | 3 or 5, chosen each round. An open-end total that is a multiple of it scores points | `Round::m_targetScore` |
| **Round score / "Current Score"** | Points a player earned so far in this round. Called "Current Score" in the save file | `Player::m_roundScore` |
| **Tournament score** | ⚠️ **Two meanings!** (1) The goal the whole session plays to (`GameState::tournamentScore`, `Tournament::m_tournamentScore`). (2) A player's running total from finished rounds (`Player::m_tournamentScore`, `PlayerState::tournamentScore`) | See [data-model](diagrams/data-model.md) |
| **Draw** | Take a tile from the boneyard because you can't play | `Boneyard::drawTile()` in `Round::takeTurn()` |
| **Pass** | Can't play and the boneyard is empty, so your turn is skipped. Two passes in a row end the round | `Round::m_consecutivePasses` |
| **First-player draw** | Each player draws one tile. Higher total starts. Both tiles go back on the bottom of the boneyard | `Round::chooseFirstPlayer()` |
| **Bonus** | End of round: the lighter hand earns (heavier − lighter) ÷ target, rounded down | `Round::finish()` |
| **Help mode** | The human types `help` or `h` and gets the computer's suggestion | `Human::chooseMove()` → `Computer::recommend()` |
| **Save file** | Plain text snapshot of a game in progress. The repo file named [`test`](../test) is one of these | `Serializer` |

### Project-specific names

| Name | Meaning |
|---|---|
| **Model / Control / Players / I/O layers** | How the author groups classes (see the [class guide PDF](../doc/class-guides/00_All-Classes.pdf)). The rule is that model classes never print or read. Only `View` does |
| **`View`** | A *namespace* (a named group of functions, not a class) that does all screen output and keyboard input |
| **`Move`** | A tiny record: a tile, an end letter, and `valid` (false means "no move") |
| **`GameState` / `PlayerState`** | Plain records holding everything in a save file |
| **`Serializer`** | Turns a `GameState` into save-file text and back. "Serialize" means convert data into text or bytes so it can be stored |
| **`recommend()`** | The computer's strategy function, shared with help mode |
| **`NO_SPINNER` (-1)** | "No spinner yet". Also reused by `getEndPips()` to mean "that end is closed" |
| **Selector / mutator** | Course terms for *getter* (reads data, marked `const`) and *setter* (changes data) |
| **`m_` prefix** | Marks a member variable (data stored inside an object), like `m_hand` |
| **Pseudocode comments** | Every function has a plain-English outline above it. Read those first! |

### C++ words you'll meet (quick definitions; the [C++ primer](cpp-primer.md) explains them with examples)

| Word | Meaning |
|---|---|
| **Header (`.h`) / source (`.cpp`)** | The `.h` file *declares* a class (what it has). The `.cpp` file *defines* it (how each function works) |
| **Include guard** | The `#ifndef TILE_H / #define TILE_H / #endif` lines. They stop a header from being pasted in twice |
| **Class / struct** | A bundle of data and functions. `struct` is the same thing but public by default. Used here for plain records |
| **`const` method** | A function that promises not to change the object |
| **`static` method** | A function that belongs to the class, not to one object. Call it as `Tile::fromString(...)` |
| **Reference (`Tile&`)** | Another name for an existing variable. Functions use it to fill in a result, like `drawTile(Tile& drawnTile)` |
| **Pointer (`Player*`)** | A variable holding the *address* of an object. `Round` holds its two players as `Player*` |
| **Inheritance** | `class Human : public Player` means a Human *is a* Player and gets everything Player has |
| **`virtual` / pure virtual (`= 0`) / `override`** | `virtual` lets a subclass replace a function. `= 0` means the base class has no version, so subclasses *must* provide one. `override` marks the replacement. This is how `Round` calls `chooseMove()` without knowing whether it's talking to a Human or a Computer (**polymorphism**) |
| **`std::vector`** | A resizable list |
| **`std::map`** | A lookup table from key to value. `GameState::players` maps a name to a `PlayerState` |
| **Copy constructor / `operator=`** | Code that runs when an object is copied. `Hand`, `Boneyard` and `Layout` define them |
| **Return by value** | `getHand()` returns a *copy*. Changing the copy does not change the player's real hand |

---

## 3. Where to find X

All code is in [`src/`](../src/): 12 `.cpp` files and 13 headers, about 2,300 lines. There are no sub-projects and no libraries.

| I want to understand… | Look in |
|---|---|
| Where the program starts | [src/main.cpp](../src/main.cpp) → `main()` creates a `Tournament` and calls `run()` |
| The whole session (resume, tournament score, rounds loop, winner) | [src/tournament.cpp](../src/tournament.cpp) `Tournament::run()` |
| The turn loop, drawing, passing, scoring | [src/round.cpp](../src/round.cpp) `Round::play()`, `Round::takeTurn()` |
| Dealing and who goes first | [src/round.cpp](../src/round.cpp) `startNew()`, `dealHands()`, `chooseFirstPlayer()` |
| End-of-round bonus | [src/round.cpp](../src/round.cpp) `Round::finish()` |
| How the human's typing becomes a move | [src/human.cpp](../src/human.cpp) `Human::chooseMove()`, `askEnd()` |
| Computer strategy / help mode | [src/computer.cpp](../src/computer.cpp) `Computer::recommend()` |
| Rules for where a tile may go | [src/layout.cpp](../src/layout.cpp) `canPlace()`, `place()`, `isOpenEnd()` |
| How points are counted on the table | [src/layout.cpp](../src/layout.cpp) `getOpenEndSum()` |
| The spinner and Up/Down arms | [src/layout.h](../src/layout.h) (header comment), `Layout::place()` |
| One tile, parsing `"2-5"` | [src/tile.cpp](../src/tile.cpp) `Tile::fromString()` |
| A player's tiles | [src/hand.cpp](../src/hand.cpp) |
| The draw pile and shuffling | [src/boneyard.cpp](../src/boneyard.cpp) |
| Shared player data (name, hand, scores) | [src/player.h](../src/player.h), [src/player.cpp](../src/player.cpp) |
| Everything printed or read | [src/view.cpp](../src/view.cpp) |
| Save-file format (write and read) | [src/serializer.cpp](../src/serializer.cpp) |
| What gets saved | [src/gamestate.h](../src/gamestate.h) |
| Model ↔ save record conversion | [src/round.cpp](../src/round.cpp) `toState()`, `restore()`, plus `Layout::toString()/fromString()` |
| An example save file | [`test`](../test) (despite the name, it's a save file, not a test) |
| Build settings | [Makefile](../Makefile), [.vscode/tasks.json](../.vscode/tasks.json) |
| Debugger settings | [.vscode/launch.json](../.vscode/launch.json) |
| The author's dev log (design decisions, bugs they hit) | [doc/log.md](../doc/log.md) |
| One-page summary of every class | [doc/class-guides/00_All-Classes.pdf](../doc/class-guides/00_All-Classes.pdf) |
| Game constants (hand size 7, targets 3/5, pips 0–6, 28 tiles) | `Round::HAND_SIZE/MIN_TARGET/MAX_TARGET` in [round.h](../src/round.h), `Tile::MIN_PIPS/MAX_PIPS` in [tile.h](../src/tile.h), `Boneyard::FULL_SET_SIZE` in [boneyard.h](../src/boneyard.h), `Tournament::MIN/MAX_TOURNAMENT_SCORE` in [tournament.h](../src/tournament.h) |

---

## 4. Build, run, and debug locally

This project is written for Unix-style tools (`make`, `g++`, a bash shell). **On Windows, use WSL** ("Windows Subsystem for Linux"). It runs a real Ubuntu Linux inside Windows, so every command in this folder works exactly as written. VS Code connects to it seamlessly.

### 4.1 One-time setup on Windows (about 60 minutes)

1. **Install WSL with Ubuntu.** Open **PowerShell as Administrator** (Start menu → type "PowerShell" → right-click → *Run as administrator*) and run:
   ```powershell
   wsl --install
   ```
   Restart when asked. After the restart an **Ubuntu** window opens and asks you to pick a Linux username and password. Remember the password, because `sudo` will ask for it.
2. **Install the compiler, make, the debugger and git** in the Ubuntu window:
   ```sh
   sudo apt update
   sudo apt install -y build-essential clang gdb git
   ```
   (`build-essential` gives you `g++` and `make`. `clang` is there because the VS Code settings point IntelliSense at `clang++`.)
3. **Get the code into your Linux home folder** (not under `C:\`: it's much faster, and it avoids Windows line-ending problems):
   ```sh
   cd ~
   git clone <repo-url> Ahmed_Hira     # your mentor gives you the URL, or copies the folder
   cd Ahmed_Hira
   ```
4. **Install VS Code on Windows** (code.visualstudio.com), then install the **WSL** extension (by Microsoft).
5. **Open the project from Ubuntu:**
   ```sh
   cd ~/Ahmed_Hira
   code .
   ```
   VS Code opens with **"WSL: Ubuntu"** in the bottom-left corner. That means it's working inside Linux.
6. When VS Code offers the **recommended extensions** (from [.vscode/extensions.json](../.vscode/extensions.json)), click *Install*. They install into WSL:
   - *C/C++* (`ms-vscode.cpptools`): code navigation
   - *CodeLLDB* (`vadimcn.vscode-lldb`): the debugger
7. *(Optional)* a Mermaid preview extension, so the diagrams render inside VS Code.

**Terminal** means the Ubuntu window, or VS Code's built-in terminal (`` Ctrl+` ``), which is already inside WSL. Windows PowerShell and Command Prompt won't work for the commands below.

> **Mentor on macOS:** install the Command Line Tools with `xcode-select --install`, open the folder in VS Code, and everything else is the same. The Makefile adds the macOS SDK flag automatically.

### 4.2 Build and run from the terminal

```sh
cd ~/Ahmed_Hira        # the repo root
make                   # compiles every src/*.cpp into ./program
./program              # or: make run
make clean             # deletes ./program (and program.dSYM on macOS)
```

A clean build prints one long `g++ ...` line and **no warnings**. If you see warnings after a change, read them: the flags `-Wall -Wextra` turn on most warnings on purpose.

`make` rebuilds whenever any `.cpp` **or `.h`** file changes. If you ever doubt whether your change was compiled, run `make clean && make`.

### 4.3 Try it (5 minutes)

**New game:** answer `n`, then `7` (tournament score), then `5` (target). Play a few turns. Type `help` at least once.

**Resume the example save:**
```
Resume a saved game? (y/n): y
Enter the save file name: test
```
The computer moves first and plays `5-6` on the Right for 5 points.

**Resume from a scenario used in this course:**
```
Enter the save file name: onboarding/saves/happy-path.txt
```
File names are relative to the folder you started the program from.

**Quit early:** press `Ctrl+D` (end of input). The program prints "Input closed. Goodbye." and exits. `Ctrl+C` also works.

**Feed answers automatically** (handy for repeating a scenario). The program reads each line as if you typed it:
```sh
printf 'y\nonboarding/saves/happy-path.txt\n5-6\nR\nn\n' | ./program
```

### 4.4 Debug in VS Code (step by step)

1. Open the project from WSL (`code .` in `~/Ahmed_Hira`), and check that the corner says "WSL: Ubuntu".
2. Open [src/round.cpp](../src/round.cpp). Click in the margin left of the line number at `Round::takeTurn` (the `while (!player->canPlay(m_layout))` line, around line 182). A red dot is a **breakpoint**: the program will pause there.
3. Open the **Run and Debug** panel (the play-with-bug icon, or `Ctrl+Shift+D`).
4. Choose **"Debug program (Windows WSL / Linux, CodeLLDB)"** from the dropdown and press the green ▶ (or `F5`).
   - It first runs the task **build (make)**, which runs `make`. The Makefile compiles with `-g`, which adds debug info so the debugger can map machine code back to your source lines.
   - The other two configurations are for macOS. Their build task uses a Mac-only path and fails on Windows.
5. The program runs in the **integrated terminal** at the bottom. **Type your answers there** (for example `y`, then `onboarding/saves/happy-path.txt`).
6. When it pauses on your breakpoint:
   - **Variables** panel: inspect `player`, `m_layout`, `m_boneyard`. Expand them.
   - `F10` **Step Over**: run this line, stop on the next.
   - `F11` **Step Into**: go inside the function called on this line.
   - `Shift+F11` **Step Out**: finish this function, return to the caller.
   - `F5` **Continue**: run until the next breakpoint.
   - `Ctrl+Shift+F5` **Restart**. `Shift+F5` **Stop**.
   - **Call Stack** panel: which functions called which to get here.
7. While paused in a function that's waiting for input (`View::readLine`), the program looks frozen. It's waiting for you to type in the terminal.

> ⚠️ **Trap:** `Ctrl+Shift+B` (the default build shortcut) runs the task *"C/C++: clang++ build active file"*. That compiles **only the file you have open**, so it fails with "undefined reference" linker errors. Use `make` in the terminal, the task **build (make)** (Terminal → Run Task…), or just press F5 with the WSL configuration selected.

---

## 5. Things that surprise newcomers (read before Day 2)

- **Tiles get flipped on the table.** Play `5-6` onto an end showing 6 and it's stored as `6-5`, so touching numbers sit next to each other. The save file shows `6-5`.
- **Getters return copies.** `player->getHand().addTile(t)` changes a temporary copy and has no effect. Use `player->addTile(t)`.
- **"Tournament Score" means two different things** in the save file, depending on whether the line is inside a player's block (see glossary).
- **Round never checks whether a player is Human or Computer.** It calls `chooseMove()` and C++ picks the right version at run time.
- **Validation happens twice on purpose.** `Human::chooseMove()` checks `Layout::canPlace()` before returning, and `Layout::place()` checks again. Model classes protect themselves.
- **The file named `test` is a save file**, not a test suite. There are no automated tests. The author tested by playing and scripting input (see [doc/log.md](../doc/log.md)).
- **Saving is offered only after a placement**, never after a pass.
- **The save-file format is fixed by the course.** Never add, remove, rename or reorder lines in it. Other programs and graders expect exactly this layout.
- **The human always draws first** when deciding who starts (the human is `m_players[0]`).
