# Day 6: Build and explain

**Goal:** Build a small feature on your own that runs through every layer of the resume flow (file → record → model → control → screen). Then explain the whole program flow back to your mentor on a whiteboard, without notes.

**Time:** about 5½ hours: ~3½ h feature, ~1 h preparing the explanation, ~45 min whiteboard session.

> **Important constraint:** the save-file format is **fixed by the course**. Your feature must not add, remove, rename or reorder any line in the save file. It only changes how strictly the program *checks* a file when resuming.

---

## What to read (30 min, refresh only)

| File | Why |
|---|---|
| Your Day 4 notes: the "break the save file" table (cases **h** and **i** were accepted but shouldn't be) and the tile-journey table | This feature fixes exactly those two gaps |
| [diagrams/resume-game.md](../diagrams/resume-game.md) | The flow you're changing |
| [src/round.cpp](../../src/round.cpp) `Round::restore()` | Where most of your change goes |
| [src/tournament.cpp](../../src/tournament.cpp) `Tournament::run()`, the `round.restore(saved)` call | Where the player is told what went wrong |
| [src/layout.h](../../src/layout.h) selectors (`getLine`, `getUpArm`, `getDownArm`) | Where tiles on the table can be read |

## Diagram / simulator for today

- [diagrams/resume-game.md](../diagrams/resume-game.md): mark every box your change touches.
- [diagrams/data-model.md §4](../diagrams/data-model.md#4-rules-the-data-must-follow-invariants): the two ❌ rows are the ones you'll turn into ✅.

---

## The feature: "Tell me why my save file is invalid"

**User story:** *As a player, when a save file can't be resumed, I want the program to tell me exactly what's wrong with it. And it should never resume a game that breaks the rules.*

Today, a broken file gives only "That save file does not describe a valid game. Starting fresh." Worse, some broken files are accepted: `Target Score: 4`, or the same tile in a hand and the boneyard.

### Acceptance criteria

Every item must be true before you're done:

1. **Target rule:** a file whose `Target Score` isn't 3 or 5 is rejected.
2. **Full-set rule:** every one of the 28 tiles (`0-0` … `6-6`) must appear **exactly once** across both hands, the layout (main line and both arms) and the boneyard. A duplicated tile or a missing tile makes the file rejected.
3. **Clear reason:** whenever `Round::restore` rejects a file (for these new rules *and* for every existing rejection), the player sees one line of the form:
   `That save file does not describe a valid game (<reason>). Starting fresh.`
   with a specific reason, for example:
   - `the target score must be 3 or 5`
   - `tile 0-5 appears more than once`
   - `tile 1-4 is missing`
   - `the layout tiles do not connect`
   - `there is no section for Human`
4. **Good files still work:** `test`, `onboarding/saves/happy-path.txt`, `draw-path.txt` and `empty-boneyard.txt` all still resume, and a game saved by the program resumes.
5. **Save format unchanged:** a game saved after your change has exactly the same lines, in the same order, as before.
6. **Layers respected:** model classes (`Layout`, `Boneyard`, `Hand`, `Tile`) print nothing. The reason is *built* in `Round` and *printed* through `View` by `Tournament`.
7. **Quality:** `make clean && make` gives **no warnings**, and new code follows the existing style (pseudocode comment above each new function, `m_` members, selectors marked `const`).

### Suggested plan (think before you type)

Before coding, write down **which file and function** you'll change for each layer, and how. Your mentor should check this list before you start (15 min).

| Layer | Question to answer |
|---|---|
| Persistence (`Serializer`, `GameState`) | Does anything here need to change, given the format is fixed? What does `load` already check, and what does it leave to `restore`? |
| Model (`Layout`) | How can `Round` get *every* tile on the table, including both arms? Is there already a selector, or should you add a small one? |
| Model (`Boneyard`, `Hand`, `Tile`) | How can you get the list of all 28 tiles without typing them out? (Hint: something in `Boneyard` already builds it.) How do you compare two tiles when `5-6` and `6-5` are the same tile? |
| Control (`Round::restore`) | How will `restore` hand a reason back to its caller? (A common C++ pattern: an extra `std::string&` output parameter, like `drawTile(Tile&)`.) In what order should the checks run? |
| Control (`Tournament::run`) | What changes at the call site? |
| View | Which existing function prints the message? |

### Test it

Make the test files first (in the repo root). Each changes one line of `test`:

```sh
sed 's/^Target Score: 5/Target Score: 4/'   test > bad-target.txt
sed 's/Hand: 1-6 1-5/Hand: 0-5 1-5/'        test > bad-duplicate.txt   # 0-5 is also in the boneyard
sed 's/^Boneyard:  1-4 /Boneyard:  /'       test > bad-missing.txt     # 1-4 disappears
sed 's/^Layout:.*/Layout:   L 0-4 5-6 R/'   test > bad-layout.txt
sed 's/^Human:/Humans:/'                    test > bad-name.txt
```

Then check each one, and that good files still load:

```sh
make clean && make
for f in bad-target.txt bad-duplicate.txt bad-missing.txt bad-layout.txt bad-name.txt \
         test onboarding/saves/happy-path.txt onboarding/saves/draw-path.txt onboarding/saves/empty-boneyard.txt; do
  echo "== $f"
  printf 'y\n%s\nn\n7\n5\n' "$f" | ./program | grep -E "Resuming|valid game"
done
```

Expected: each `bad-*` file prints its specific reason, and each good file prints "Resuming the saved round".

Format check (criterion 5): resume `happy-path.txt`, play `5-6` `R`, save to `after.txt`, and compare its line labels with `test`. Only the tiles and numbers should differ.

---

## Prepare and give the whiteboard explanation

### Prepare (60 min)

Without looking at the diagrams, draw on paper:

1. The **architecture**: all classes grouped by layer, with arrows.
2. **One human turn**, as a sequence: every `Class::method` from `Round::play` to the points message.
3. **Save and resume**: the path from the model to the file and back, and which checks happen in `load` vs `restore`.
4. **Your feature**: the resume diagram with your changes highlighted.

Then compare with [architecture.md](../diagrams/architecture.md), [request-flow.md](../diagrams/request-flow.md), [save-game.md](../diagrams/save-game.md) and [resume-game.md](../diagrams/resume-game.md), and fix any gaps.

### Present (30–45 min, with your mentor)

Explain on a whiteboard, in this order:

1. What the program does, in two minutes, as if to a new player.
2. The architecture and the two design rules (all I/O in `View`, all file access in `Serializer`).
3. One human turn end to end, including one validation failure and the draw/pass path.
4. How the computer decides, and why help mode gives the same answer.
5. Save and resume, and the difference between a *format* check and a *game-rule* check.
6. Your feature: walk through your diff layer by layer, and show the tests passing.

Your mentor will ask "what happens if…?" questions. Answer them by pointing at the code path on your diagram.

---

## Check questions

1. List every file you changed for the feature, and say which layer each belongs to. Which layers did you *not* need to change, and why?
2. Why do the new checks belong in `Round::restore` rather than in `Serializer::load`?
3. How did you get the reason text from `Round::restore` to the screen without making `Round` print anything?
4. Why is comparing tiles with `==` (and not by comparing `getLeftPips()` and `getRightPips()` directly) important for the full-set check?
5. `restore` builds a local `Layout` and `Boneyard`, and only copies them into the round at the very end. Why does that matter now that there are more ways for it to fail?

---

---

## Answer key

These are reference answers. Other correct designs exist; judge by the acceptance criteria. Your mentor has a reference implementation to compare with.

1. Typically: `layout.h`/`layout.cpp` (model: a small `getAllTiles()` selector, optional, because `getLine`/`getUpArm`/`getDownArm` also work), `round.h`/`round.cpp` (control: `restore(const GameState&, std::string& reason)` with the new checks and a reason on every `return false`), `tournament.cpp` (control: pass a `reason` string and include it in the message). **Not changed:** `serializer.cpp` and `gamestate.h`, because the format is fixed and the record already carries everything needed. `view.cpp`, because `View::message` already prints any line.
2. `Serializer` only knows the *format* (lines, numbers, tile text). "3 or 5" and "28 tiles exactly once" are *game rules*, and `Round` owns the rules and the constants (`MIN_TARGET`, `MAX_TARGET`). Keeping it this way means the same checks would protect any other source of a `GameState`, not just files.
3. An output parameter: `restore` fills a `std::string& reason`, the same pattern as `Boneyard::drawTile(Tile&)`. `Tournament::run` (a control class that already talks to `View`) builds the sentence and calls `View::message`. The model and `Round::restore` stay free of I/O.
4. Tiles in hands and the boneyard are stored as dealt (like `5-6`), while tiles on the table are *flipped* to fit (like `6-5`). `Tile::operator==` treats both orientations as the same domino. Comparing the pips directly would report `5-6` as missing and `6-5` as an unknown extra.
5. It's "validate, then commit": if any check fails, `restore` returns before touching the round's real `m_layout`, `m_boneyard` or the players' hands and scores. The failed attempt leaves no half-restored state, and the "Starting fresh" path begins from a clean round.
