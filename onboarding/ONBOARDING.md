# Onboarding: 35 Domino (C++)

Welcome! This folder teaches you this codebase in 6 days. By the end you should be able to:

1. explain what the program does and how a turn flows through the code,
2. trace one turn through every class in a debugger, and
3. make a small change on your own.

**How to use this folder**

| Item | What it is |
|---|---|
| [`site/index.html`](site/index.html) | **The whole course as web pages. Start here each day.** Open it with `start onboarding/site/index.html`. It has a home page with your progress, one page per day, checklists and answer boxes that are remembered, and code links that open in VS Code |
| `ONBOARDING.md` (this file) | The overview: what the app does, vocabulary, where things live, how to build/run/debug |
| [`cpp-primer.md`](cpp-primer.md) | How basic programming ideas are written in C++, using examples from this code. Part 1 on Day 1, Part 2 on Day 2 |
| [`diagrams/`](diagrams/) | Mermaid diagrams. They render on GitHub, and in VS Code with a Mermaid preview extension |
| [`simulator.html`](simulator.html) | Open it in any browser. Step through one turn, class by class, with sample data |
| [`days/`](days/) | The 6-day plan. Do one file per day |
| [`tools/build_site.py`](tools/build_site.py) | For the mentor: rebuilds `site/` after any `.md` file changes (`python3 onboarding/tools/build_site.py`) |
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
> | Deployable | One executable file: `program.exe` on Windows, `program` on Mac/Linux |

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

On Windows you'll install three free tools:

| Tool | What it gives you |
|---|---|
| **Git for Windows** | `git` to download the code, plus **Git Bash**, a terminal where every command in this course works (`grep`, `printf`, `sed`, `./program`) |
| **MSYS2** | The C++ compiler (`g++`), the debugger (`gdb`) and `make` |
| **VS Code** | The editor, with a built-in terminal and debugger |

### 4.1 One-time setup on Windows (about 45 minutes)

**Step 1: install Git for Windows.** Download it from https://git-scm.com/download/win and run the installer. Accept all the default options.

**Step 2: install MSYS2.** Download the installer from https://www.msys2.org and run it. Keep the default folder, **`C:\msys64`**, because the project's settings expect it.

> ⏳ **The last part of the installer can take up to 15 minutes and may look frozen.** Don't close it. Wait until it says it's finished.

**Step 3: install the compiler, debugger and make.**
1. From the Start menu, open **"MSYS2 UCRT64"**. It must be the one with *UCRT64* in the name. A black terminal window opens.
2. Type this and press Enter. Press Enter again to accept, and `Y` to confirm:
   ```sh
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-make
   ```
   Red lines like `error: failed retrieving file '…' from mirror.msys2.org : Connection timed out` are **normal** if pacman then carries on to `installing …`: it just tried another download server. If it **stops** with an error instead, run the same command again.
3. Then run this one line. MSYS2 names its make tool `mingw32-make`, and this copy lets you type plain `make` like the rest of the course:
   ```sh
   cp /ucrt64/bin/mingw32-make.exe /ucrt64/bin/make.exe
   ```
4. Close the MSYS2 window. You won't need it again.

**Step 4: let Windows find the tools.**
1. Start menu → type **"environment"** → click **"Edit environment variables for your account"**.
2. In the top list, select **Path** → **Edit…** → **New** → type `C:\msys64\ucrt64\bin`.
3. With the new line still selected, click **Move Up** until it's at the **top** of the list. This matters if the computer already has another compiler installed: Windows uses the first one it finds.
4. Click **OK**, then **OK** again.
5. **Close every open VS Code and Git Bash window.** Programs that were already open keep the old Path and won't find the new tools.

**Step 5: install VS Code** from https://code.visualstudio.com (default options).

**Step 6: check the tools, then download the code.** From the Start menu, open **Git Bash**. Open a *new* one, so it picks up the Path change from step 4. Then run:
```sh
g++ --version      # should print a version number
gdb --version
make --version
which g++ gdb make # all three lines must start with /c/msys64/ucrt64/bin
cd ~
git clone https://github.com/rome8721/Ahmed_Hira.git
cd Ahmed_Hira
code .
```
Run these one line at a time. If a `--version` line says "command not found", step 4 didn't take effect: recheck the Path entry, then close and reopen Git Bash. If `which` shows a different folder, another compiler is ahead of MSYS2 in the Path: go back to step 4 and move `C:\msys64\ucrt64\bin` to the top.

**Step 7: in VS Code.**
1. If VS Code asks whether you trust the authors of the folder, choose **Yes**.
2. Install the **C/C++** extension (by Microsoft). It's the only one you need. VS Code usually shows a pop-up offering the project's recommended extension: click **Install**. If you missed or closed the pop-up, press `Ctrl+Shift+X`, type `@recommended` in the search box, and click **Install** on **C/C++**.
3. Open the terminal: press `` Ctrl+` `` (the key just below `Esc`). Or use the menu: **Terminal → New Terminal**. If you don't see a menu bar, click the **☰** icon at the top left first.
   Check the prompt: it should contain **MINGW64** in purple, which means Git Bash. The project tells VS Code to use Git Bash. If the prompt starts with `PS C:\` instead (PowerShell), click the **˅** next to the **+** in the terminal panel and choose **Git Bash**.
4. *(Optional)* install a Mermaid preview extension, so the diagrams render inside VS Code.

From now on, **"the terminal"** means Git Bash, either inside VS Code (best) or the Git Bash window. PowerShell and Command Prompt won't understand the commands in this course.

> **Mentor on macOS or Linux:** install a compiler (macOS: `xcode-select --install`; Ubuntu: `sudo apt install build-essential gdb`), open the folder in VS Code, install the **CodeLLDB** extension (`vadimcn.vscode-lldb`), and use the matching *macOS* or *Linux* debug configuration. The Makefile adapts to each system automatically.

### 4.2 Build and run from the terminal

Run these **one line at a time**. `./program` starts the game and waits for your answers, so don't paste them all at once.

```sh
cd ~/Ahmed_Hira        # the repo root
make                   # compiles every src/*.cpp into program.exe
./program              # runs it (Windows finds program.exe). Or: make run
```

Later, when you want to throw away the compiled program and rebuild from scratch, use `make clean`. **Not now**: section 4.3 needs the program you just built.

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

**Quit early:** press `Ctrl+C`.

**Feed answers automatically** (handy for repeating a scenario). The program reads each line as if you typed it:
```sh
printf 'y\nonboarding/saves/happy-path.txt\n5-6\nR\nn\n' | ./program
```

**Open the course pages** (the easiest way to follow the six days): `start onboarding/site/index.html`, or double-click `site\index.html` in the `onboarding` folder.

**Open the simulator:** in File Explorer go to your `Ahmed_Hira\onboarding` folder and double-click `simulator.html`, or run `start onboarding/simulator.html` in Git Bash.

### 4.4 Debug in VS Code (step by step)

1. Open [src/round.cpp](../src/round.cpp). Click in the margin left of the line number at `Round::takeTurn` (the `while (!player->canPlay(m_layout))` line, around line 182). A red dot is a **breakpoint**: the program will pause there.
2. Open the **Run and Debug** panel (the play-with-bug icon, or `Ctrl+Shift+D`).
3. Choose **"Debug program (Windows, gdb)"** from the dropdown and press the green ▶ (or `F5`).
   - It first runs the task **build (make)**, which runs `make`. The Makefile compiles with `-g`, which adds debug info so the debugger can map machine code back to your source lines.
   - The *Linux* and *macOS* configurations don't work on Windows.
4. **A separate console window opens.** That's your program. **Type your answers in that window** (for example `y`, then `onboarding/saves/happy-path.txt`). VS Code's own panels only show the debugger.
5. When it pauses on your breakpoint (VS Code comes to the front):
   - **Variables** panel: `player` is listed directly. The round's own data (`m_layout`, `m_boneyard`, `m_next` and the rest) is inside **`this`**: click the arrow next to `this` to expand it.
   - Don't be alarmed by nonsense values such as `points = 16777216`. Variables declared further down the function, like `sum`, `points` and `note`, hold leftover memory until their line runs.
   - `F10` **Step Over**: run this line, stop on the next.
   - `F11` **Step Into**: go inside the function called on this line.
   - `Shift+F11` **Step Out**: finish this function, return to the caller.
   - `F5` **Continue**: run until the next breakpoint.
   - `Ctrl+Shift+F5` **Restart**. `Shift+F5` **Stop**.
   - **Call Stack** panel: which functions called which to get here.
6. While the program waits for input (`View::readLine`), nothing happens in VS Code. Switch to the console window and type.

> ⚠️ **Trap:** `Ctrl+Shift+B` (the default build shortcut) runs the task *"C/C++: clang++ build active file"*, which is set up for Mac and fails on Windows. Build with `make` in the terminal, with the task **build (make)** (**Terminal → Run Task…**, via **☰** if the menu bar is hidden), or just press F5 with the Windows configuration selected.

> **If F5 says it can't find gdb:** MSYS2 isn't installed in `C:\msys64`. Either reinstall it there, or change `miDebuggerPath` in [.vscode/launch.json](../.vscode/launch.json) to where `gdb.exe` actually is.

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
