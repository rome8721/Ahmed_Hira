# Day 1: The story and the map

**Goal:** By the end of today you have the project running on your Windows machine, you can say in two minutes what the program does, and you can name every class and its job.

**Time:** about 5½ hours.

| Block | Time |
|---|---|
| Setup: Git, compiler, VS Code | 60 min |
| The story: read ONBOARDING, play the game | 60 min |
| C++ primer, Part 1 | 75 min |
| The map: architecture, class guide, header tour | 75 min |
| Exercises and check questions | 60 min |

---

## What to read

| Order | File | Why | Time |
|---|---|---|---|
| 1 | [ONBOARDING.md §4.1–4.3](../ONBOARDING.md#4-build-run-and-debug-locally) | Install the tools and build the program | 60 min (doing) |
| 2 | [ONBOARDING.md](../ONBOARDING.md) §1 (what the app does) and §2 (glossary). Skim the C++ words table; the primer explains them properly | The story and the vocabulary | 30 min |
| 3 | [cpp-primer.md](../cpp-primer.md) **Part 1** (sections 1–7) | How the programming ideas you know are written in C++ | 75 min |
| 4 | [diagrams/architecture.md](../diagrams/architecture.md), diagram 1 only | The map | 20 min |
| 5 | [doc/class-guides/00_All-Classes.pdf](../../doc/class-guides/00_All-Classes.pdf), page 1 (the box diagram at the top, then Tile and Hand) | The author's summary of each class | 20 min |
| 6 | **Only the comment at the top** of each header in [src/](../../src/) (the first 2–4 lines): `tile.h`, `move.h`, `hand.h`, `boneyard.h`, `layout.h`, `player.h`, `human.h`, `computer.h`, `round.h`, `tournament.h`, `gamestate.h`, `serializer.h`, `view.h` | Each says in plain English what the class is for | 20 min |
| 7 | [src/tile.h](../../src/tile.h) and [src/tile.cpp](../../src/tile.cpp), in full | The smallest class, fully explained by the primer. Your first complete read of real code | 15 min |

## Diagram / simulator for today

- [diagrams/architecture.md](../diagrams/architecture.md), diagram 1.
- Don't open the simulator yet. That's Day 2.

---

## Hands-on exercises

### 1. Set up, build and run (60 min)

Follow [ONBOARDING.md §4.1](../ONBOARDING.md#41-one-time-setup-on-windows-about-45-minutes) step by step. Ask your mentor if anything fails. Setup problems are normal, and they aren't the point of the day.

Then build and run, one line at a time:

```sh
cd ~/Ahmed_Hira
make
./program
```

- [ ] The build prints one `g++ ...` line and **no warnings**.
- [ ] VS Code's terminal prompt shows **MINGW64** (Git Bash), not `PS C:\` (PowerShell).
- [ ] `start onboarding/site/index.html` opens the course home page in your browser. From now on, you can follow each day there.
- [ ] `start onboarding/simulator.html` opens the simulator (just check it opens; you'll use it on Day 2).

### 2. Play the game (30 min)

- [ ] Start a **new game**: tournament score `7`, target `5`. Play until the round ends. Use `help` at least twice.
- [ ] Start again and **resume** the file `test`. Watch the computer play `5-6`.
- [ ] Play one turn, answer `y` to "Save the game and quit?", save to `day1.txt`, and open that file in VS Code.
- [ ] Resume `day1.txt`. You're exactly where you stopped.

### 3. Who prints what? (20 min)

Pick five different *prompts* the program showed you (for example "Enter the target score for this round (3 or 5):"). Find each one in the code:

```sh
grep -n "target score for this round" src/*.cpp
```

(`grep` searches files for text. `-n` shows line numbers.) Write down: prompt → file → function.

### 4. Prove the "only View does I/O" rule (10 min)

```sh
grep -n "cout\|cin\|getline" src/*.cpp
grep -n "fstream" src/*.cpp
```

Which files show up? Is that what `architecture.md` claims?

### 5. Read Tile like a pro (20 min)

With [cpp-primer.md](../cpp-primer.md) Part 1 open, go through [src/tile.cpp](../../src/tile.cpp) function by function. For each one, write one sentence in your own words saying what it does. Then answer:
- What does `Tile(3, 8)` become, and why?
- Why does `operator==` say `2-5` equals `5-2`?

### 6. Draw the map from memory (15 min)

Close everything. On paper, draw boxes for the classes and arrows for "uses". Group them into Control / Players / Model / Persistence / I/O. Then compare with [architecture.md](../diagrams/architecture.md) and fix your drawing in a different colour.

---

## Check questions

1. In one or two sentences, what does this program do for a player?
2. Which class is the only one allowed to print text or read the keyboard? Which is the only one that reads or writes files?
3. What is the boneyard, and which end of it do players draw from?
4. The file named `test` in the repo root: what is it, and how do you use it?
5. In C++, what is the difference between `tile.h` and `tile.cpp`? Why do you need to run `make` after changing either one?

---

---

## Answer key

**Exercise 5:** `Tile(3, 8)` becomes **0-0**. The constructor starts at 0-0 and calls `setPips(3, 8)`, which refuses because 8 is outside 0–6, so nothing changes. `operator==` checks both "same way round" (`sameWay`) and "flipped" (`flippedWay`), and returns true if either matches, because a physical domino is the same piece whichever way you hold it.

**Check questions**

1. It's a terminal dominoes game ("35 Domino") where one human plays the computer. Players place matching tiles and score when the open ends add up to a multiple of 3 or 5, over several rounds until someone reaches the tournament score. Games can be saved to a text file and resumed.
2. **`View`** ([src/view.cpp](../../src/view.cpp)) does all printing and reading. **`Serializer`** ([src/serializer.cpp](../../src/serializer.cpp)) is the only code that opens files. Your `grep` in exercise 4 should confirm this.
3. The face-down pile of tiles not yet dealt (`Boneyard`). Draws come off the **front**, the left-most tile in the save file (`Boneyard::drawTile`). Tiles returned after the first-player draw go on the **bottom**, at the end (`addToBottom`).
4. Despite the name, it's a **save file**, not a test. Start `./program`, answer `y` to "Resume a saved game?", and type `test` as the file name.
5. `tile.h` (the header) *declares* what a `Tile` has: its functions and data. `tile.cpp` *defines* how each function works. The computer only runs the compiled file `program`, so changes to either file do nothing until `make` recompiles.
