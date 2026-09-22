# Day 5: Break and fix

**Goal:** Find and fix three realistic bugs using only the symptoms, the diagrams and the debugger. Practise the loop you'll use every day as a developer: **reproduce → locate → understand → fix → verify**.

**Time:** about 5 hours (about 1 hour per bug, plus setup and review).

---

## Setup (20 min)

Your mentor will give you a branch called `day5-bugs` with three bugs planted in it. You don't know where they are. That's the point.

```sh
git switch day5-bugs
make clean && make
```

> Fetch the branch first if you don't have it: `git fetch origin && git switch day5-bugs`. The tickets below describe symptoms only.

**Rules for today**

1. Use the hints under each ticket only after 45 minutes stuck on a bug, and open them one at a time. Your mentor has the full solutions.
2. For every bug, write down: **how you reproduced it**, **the file:line of the cause**, **why it was wrong**, and **how you verified the fix**.
3. Fix the cause, not the symptom. For example, don't "fix" a crash by wrapping `main()` in a try/catch.

## What to read / use

| Resource | Use it for |
|---|---|
| [diagrams/request-flow.md](../diagrams/request-flow.md), [save-game.md](../diagrams/save-game.md), [resume-game.md](../diagrams/resume-game.md), [new-round.md](../diagrams/new-round.md) | Narrowing down *which layer* a symptom comes from |
| [simulator.html](../simulator.html) | The correct data at each step, to compare against the debugger |
| [onboarding/saves/](../saves/) | Reproducing exact situations |
| Your Day 4 "one tile's journey" table | Bug 1 |

---

## Bug ticket #1: "My points disappear when I save"

> **Reported by:** a player
> **Steps:** Resume `onboarding/saves/happy-path.txt`. Play `5-6` on `R`. The game says "5 points!". Save to `bug1.txt` and quit.
> **Expected:** in `bug1.txt`, the Human's `Current Score` is 5 and the Computer's is 10.
> **Actual:** both players' `Current Score` lines say 0. After resuming, the points are gone.
> **Category:** wrong mapping

**Hints (read one at a time, only if stuck)**

<details><summary>Hint 1</summary>The score is right on screen, so the model is fine. The bug is somewhere between the model and the file. Which functions sit in that gap? (See save-game.md.)</details>
<details><summary>Hint 2</summary>Open the saved file. What value would produce <em>exactly</em> 0 for both players in this scenario? Which other field in the file is 0 for both?</details>
<details><summary>Hint 3</summary>Put a breakpoint in <code>Round::toState</code> and look at each <code>ps</code> field just before <code>state.players[...] = ps;</code>.</details>

---

## Bug ticket #2: "The game crashes when I can't play late in a round"

> **Reported by:** a player
> **Steps:** Resume `onboarding/saves/empty-boneyard.txt`. It's the Human's turn and none of the Human's tiles fit.
> **Expected:** "Human cannot play and the boneyard is empty. Human passes." Then the Computer plays.
> **Actual:** the program dies. In the terminal: `Segmentation fault (core dumped)` (exit code 139, check with `echo $?`). Run from VS Code, the debugger should stop at the crash, reported as a `SIGSEGV` signal (macOS calls it `EXC_BAD_ACCESS`).
> **Category:** missing null / empty check

**Jargon:** a **segmentation fault** ("segfault") means the program touched memory it doesn't own. In C++, a common cause is using something that doesn't exist: dereferencing a null pointer, or reading `front()` of an empty `std::vector`. C++ doesn't check for you. The code has to.

**Hints**

<details><summary>Hint 1</summary>Run it under the debugger (F5). When it crashes, the <strong>Call Stack</strong> panel shows where. Click the frames from the top down until you reach code in <code>src/</code>.</details>
<details><summary>Hint 2</summary>Look at the diagram in round-end.md: which call is supposed to <em>return false</em> when there's nothing left?</details>
<details><summary>Hint 3</summary>Compare that function with its neighbour <code>peekFront()</code>. What does <code>peekFront</code> check that the crashing function doesn't?</details>

**Side lesson:** if you run it with piped input (`printf 'y\n...' | ./program`), you might see *less* output before the crash than when you type interactively. When output goes to a pipe, it's buffered and never flushed when the program crashes. Don't trust "the last line printed" as the crash location. Trust the debugger.

---

## Bug ticket #3: "I can't choose target 5 any more"

> **Reported by:** a tester
> **Steps:** Start a new game (`n`), tournament score `7`, then type `5` for the target.
> **Expected:** the round starts with target 5.
> **Actual:** `Please enter a whole number from 3 to 4.` Only 3 is ever accepted. Oddly, **resuming** `happy-path.txt` (target 5) still works.
> **Category:** bad config value

**Hints**

<details><summary>Hint 1</summary>Search for the text "whole number from". Where do the numbers 3 and 4 in that message come from?</details>
<details><summary>Hint 2</summary>Follow the arguments back from <code>View::readInt</code> to its caller in <code>Tournament::run</code>. They're named constants. Where are they defined?</details>
<details><summary>Hint 3</summary>Why does resume still work? Look at where <code>Round::restore</code> gets <code>m_targetScore</code> from.</details>

---

## After all three: review (30 min)

1. Run all three reproductions again on your fixed branch and confirm they pass.
2. Run the happy path and the draw path once more to confirm nothing else broke (regression check).
3. `git diff main` should show exactly three small changes. Compare with your mentor.
   Tip: after editing a header such as `round.h`, `make` rebuilds automatically, because the Makefile watches `.h` files too. If a fix ever seems to have no effect, run `make clean && make` to be sure.
4. Write 3–5 sentences: which bug was hardest to *locate*, and which tool helped most?

---

## Check questions

1. For bug #1, why did the on-screen score look correct even though the saved file was wrong? Which layer boundary was broken?
2. For bug #2, why might a crash like this *not* always happen, even though the code is wrong? (Undefined behaviour.)
3. For bug #3, why did resuming a target-5 game keep working?
4. What's the difference between the *symptom*, the *location* and the *root cause* of a bug? Answer using bug #2.
5. Name one way the codebase could have caught each bug earlier.

---

---

## Answer key

1. The model (`Player::m_roundScore`) was correct, and `View` reads the model, so the screen was right. The bug was in the **model → record** conversion (`Round::toState`), which only runs when saving. Data crossing a boundary (entity → DTO) is a classic place for mapping bugs.
2. Calling `front()` or `erase(begin())` on an empty `std::vector` is **undefined behaviour**: C++ promises nothing. It may crash, return garbage, or appear to work, depending on memory layout, compiler and optimisation level. Here it reliably segfaults, but you can't rely on that. That's why the explicit `isEmpty()` guard matters.
3. `Round::restore` copies `m_targetScore` straight from the save file (`state.targetScore`) and never compares it against `MIN_TARGET`/`MAX_TARGET`. Only the *new round* path in `Tournament::run` uses those constants.
4. **Symptom:** segfault when a player can't play and the boneyard is empty. **Location:** `Boneyard::drawTile`, which reads `m_tiles.front()` without checking. **Root cause:** a missing precondition check. `Round::takeTurn` relies on `drawTile` returning `false` when empty to detect a pass, and that contract was broken.
5. Examples: (1) an automated save → load round-trip test that compares scores; (2) a unit test calling `drawTile` on an empty `Boneyard`, or building with `-fsanitize=address`, or with `_LIBCPP_ENABLE_ASSERTIONS`/`-D_GLIBCXX_ASSERTIONS` so bad container access fails loudly; (3) a `static_assert(MAX_TARGET == 5)`, a test of the target prompt, or having the prompt text use the constants so the mismatch is visible.
