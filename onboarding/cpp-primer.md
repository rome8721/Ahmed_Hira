# C++ primer: just enough to read this codebase

You know basic programming logic: variables, `if`, loops, functions. This primer shows how those ideas are **written in C++**, and then adds the few C++ features this project uses. Every example comes from the real code in `src/`, so each section also teaches you part of the game.

You don't need to memorise this. Read it once, then come back when a line of code confuses you.

- **Part 1 (Day 1):** sections 1–7, about 75 minutes
- **Part 2 (Day 2):** sections 8–13, about 60 minutes

Try the small "Predict" questions as you go. Answers are at the bottom.

---

# Part 1: the basics, written in C++

## 1. What a C++ program is

- The code is in **text files**. `.cpp` files hold the code that runs. `.h` files ("headers") hold *declarations*: a list of what exists, so other files can use it.
- A **compiler** (`g++`) turns all the `.cpp` files into one runnable file. Here that's `program.exe` on Windows (`program` on Mac and Linux). You run `make`, which runs `g++` for you (see the [Makefile](../Makefile)).
- Execution always starts at a function called `main`. Here it's in [src/main.cpp](../src/main.cpp):

```cpp
#include "tournament.h"      // "paste in" the declarations from tournament.h

int main() {
    Tournament tournament;   // create a Tournament object
    tournament.run();        // run the whole game
    return 0;                // 0 means "finished normally"
}
```

- `//` starts a comment. The compiler ignores it.
- Every statement ends with `;`. Blocks of code are wrapped in `{ }`.
- If you change code, you must **recompile** (`make`) before running again.

## 2. Variables and types

C++ wants to know the **type** of every variable when you create it.

| Type | Holds | Example from the code |
|---|---|---|
| `int` | whole numbers | `int total = 0;` |
| `bool` | `true` or `false` | `bool placed = takeTurn(current);` |
| `char` | one character, in single quotes | `char end = 'R';` |
| `std::string` | text, in double quotes | `std::string answer = "5-6";` |

```cpp
static const int HAND_SIZE = 7;     // round.h. const = can never change
```

`const` means "this value can't change after it's set". Names in `ALL_CAPS` are constants by convention.

## 3. Functions

A function has a **return type**, a **name**, **parameters** in `( )`, and a body in `{ }`:

```cpp
// tile.cpp
bool Tile::isValidPips(int pips) {
    return pips >= MIN_PIPS && pips <= MAX_PIPS;
}
```

This reads as: "a function named `isValidPips` that takes an `int` called `pips` and returns a `bool`." `void` as a return type means the function returns nothing.

(The `Tile::` part means "this function belongs to the `Tile` class". See section 7.)

## 4. Decisions: `if`, `else`, and operators

```cpp
// tile.cpp: Tile::setPips
if (!isValidPips(leftPips) || !isValidPips(rightPips)) {
    return false;
}
m_leftPips = leftPips;
m_rightPips = rightPips;
return true;
```

| Operator | Meaning |
|---|---|
| `==` `!=` | equal / not equal (**two** `=` for comparing; one `=` assigns) |
| `<` `<=` `>` `>=` | comparisons |
| `&&` | and |
| `\|\|` | or |
| `!` | not |
| `%` | remainder: `15 % 5` is 0, `11 % 5` is 1 |

There's also a one-line "if": `condition ? valueIfTrue : valueIfFalse`. It's used for scoring in [round.cpp](../src/round.cpp):

```cpp
int points = (sum % m_targetScore == 0) ? sum : 0;   // score the sum if it divides evenly
```

**Predict A:** with `sum = 11` and `m_targetScore = 5`, what is `points`?

## 5. Loops

**Counting loop** ([boneyard.cpp](../src/boneyard.cpp), building all 28 tiles):

```cpp
for (int left = 0; left <= 6; left++) {          // start; keep going while true; step
    for (int right = left; right <= 6; right++) {
        m_tiles.push_back(Tile(left, right));    // add tile left-right to the list
    }
}
```

**"For each" loop** ([hand.cpp](../src/hand.cpp)). Read it as "for each tile in m_tiles":

```cpp
int total = 0;
for (const Tile& tile : m_tiles) {
    total += tile.getSum();          // same as: total = total + tile.getSum();
}
```

**`while` loop, with `continue` and `return`** ([human.cpp](../src/human.cpp), simplified):

```cpp
while (true) {                                   // loop forever...
    std::string answer = View::readWord("Enter the tile to play: ");
    if (!Tile::fromString(answer, tile)) {
        View::message("That is not a tile.");
        continue;                                // ...skip to the next time round
    }
    // ... more checks ...
    return Move(tile, end);                      // ...until we return a good answer
}
```

`continue` jumps back to the top of the loop. `break` leaves the loop. `return` leaves the whole function.

**Predict B:** how many tiles does the nested `for` loop above create? (Hint: when `left` is 0, `right` goes 0..6, so 7 tiles. When `left` is 1…)

## 6. Lists and text: `std::vector` and `std::string`

`std::vector<Tile>` means "a list of `Tile`s that can grow and shrink". The type inside `< >` is what the list holds.

| Code | Meaning |
|---|---|
| `m_tiles.push_back(t)` | add `t` to the end |
| `m_tiles.size()` | how many items |
| `m_tiles.empty()` | `true` if there are none |
| `m_tiles[i]` | item at position `i`. **Positions start at 0** |
| `m_tiles.front()` | first item (the list must not be empty!) |
| `m_tiles.erase(m_tiles.begin() + i)` | remove the item at position `i` |

```cpp
// boneyard.cpp: take the first tile off the pile
bool Boneyard::drawTile(Tile& drawnTile) {
    if (isEmpty()) {
        return false;                   // nothing to draw
    }
    drawnTile = m_tiles.front();        // copy the first tile out
    m_tiles.erase(m_tiles.begin());     // remove it from the pile
    return true;
}
```

Strings work much the same: `text.length()`, `text[1]` (one `char`), and `+` to join: `"Human" + std::string(" passes.")`. `std::to_string(5)` turns a number into text.

## 7. Classes and objects

A **class** bundles data together with the functions that work on that data. A **Tile** has two numbers and some functions. Here's [tile.h](../src/tile.h), shortened:

```cpp
class Tile {
public:                                  // anyone may use these
    Tile(int leftPips = 0, int rightPips = 0);   // constructor: runs when a Tile is created
    int getLeftPips() const;             // "selector" (getter)
    int getSum() const;
    bool isDouble() const;
    bool setPips(int leftPips, int rightPips);   // "mutator" (setter)

private:                                 // only Tile's own functions may touch these
    int m_leftPips;                      // m_ = "member": data stored inside each Tile
    int m_rightPips;
};
```

- The **class** is the blueprint. An **object** is one actual thing built from it: `Tile t(2, 5);` creates a tile 2-5.
- Call a function on an object with a dot: `t.getSum()` gives 7, and `t.isDouble()` gives false.
- **`private`** data can only be changed through the class's own functions. That's how `Tile` guarantees its pips are always 0–6: the only way in is `setPips`, which checks.
- The **header (`.h`)** lists what the class has. The **`.cpp`** file writes each function out, with the `ClassName::` prefix:

```cpp
// tile.cpp
Tile::Tile(int leftPips, int rightPips) : m_leftPips(0), m_rightPips(0) {
    setPips(leftPips, rightPips);
}
```

The part after `:` is an **initializer list**. It sets the member variables before the body runs.

**Predict C:** `Tile t(4, 9);` Pips must be 0–6, so what does `t.toString()` return? (Read `Tile::Tile` and `setPips` above.)

**You now know enough for Day 1.** Part 2 is for Day 2.

---

# Part 2: the C++ features this project relies on

## 8. `const` functions and `static` functions

```cpp
int getSum() const;                                      // const: promises not to change the object
static bool fromString(const std::string& text, Tile& result);   // static: belongs to the class
```

- A **`const` function** only reads. The compiler stops it from changing any `m_` variable.
- A **`static` function** doesn't need an object. You call it on the class name: `Tile::fromString("5-6", tile)`. It's like a plain function kept inside the class for tidiness.

## 9. References (`&`): "another name for the same variable"

Normally, passing a variable to a function **copies** it. A reference (`Type&`) passes the variable *itself*, so the function can change it:

```cpp
Tile drawn;                          // round.cpp
if (m_boneyard.drawTile(drawn)) {    // drawTile(Tile& drawnTile) fills in 'drawn'
    player->addTile(drawn);
}
```

This project uses references in two ways:

| Pattern | Example | Why |
|---|---|---|
| `Type&`: an **output** parameter | `bool drawTile(Tile& drawnTile)` | return true/false **and** hand back a tile |
| `const Type&`: a **read-only** parameter | `bool canPlay(const Layout& layout) const` | avoid copying a big object, and promise not to change it |

## 10. Pointers (`*`) and `->`

A **pointer** holds the *address* of an object: where it lives in memory. The `Round` doesn't own the players. It just needs to point at them:

```cpp
// tournament.cpp: &m_human means "the address of m_human"
Round round(&m_human, &m_computer, target);

// round.h
Player* m_players[2];             // an array of 2 pointers-to-Player

// round.cpp: use -> (not .) to call a function through a pointer
Player* current = m_players[m_next];
current->getName();
```

Rule of thumb: an **object** uses `.`, a **pointer** uses `->`.

## 11. Inheritance and `virtual`: one call, two behaviours

`Human` and `Computer` both **inherit from** `Player`. They get its name, hand and scores for free, and each supplies its own way of choosing a move.

```cpp
// player.h
class Player {
public:
    virtual Move chooseMove(const Layout& layout, int targetScore) = 0;   // "= 0": no version here
protected:                  // like private, but subclasses may use it too
    Hand m_hand;
};

// human.h
class Human : public Player {           // "a Human is a Player"
public:
    Move chooseMove(const Layout& layout, int targetScore) override;   // asks the keyboard
};

// computer.h
class Computer : public Player {
public:
    Move chooseMove(const Layout& layout, int targetScore) override;   // uses the strategy
};
```

Now the key line in [round.cpp](../src/round.cpp):

```cpp
Move move = player->chooseMove(m_layout, m_targetScore);
```

`player` is a `Player*`. Because `chooseMove` is **`virtual`**, C++ checks *at run time* what the object really is, and runs `Human::chooseMove` or `Computer::chooseMove`. This is called **polymorphism**. It lets `Round` treat both players identically.

## 12. `struct`, `enum`, `namespace`, `std::map`

| Feature | Example | Meaning |
|---|---|---|
| `struct` | `struct Move { Tile tile; char end; bool valid; };` ([move.h](../src/move.h)) | A class whose members are public by default. Used for simple records |
| `enum` | `enum Outcome { FINISHED, SAVE_REQUESTED };` ([round.h](../src/round.h)) | A fixed set of named values |
| `namespace` | `namespace View { void message(...); }` ([view.h](../src/view.h)) | A named group of plain functions. Call as `View::message("hi")` |
| `std::map` | `std::map<std::string, PlayerState> players;` ([gamestate.h](../src/gamestate.h)) | A lookup table: `players["Human"]` gives the Human's saved data |
| `#ifndef X / #define X / #endif` | top and bottom of every `.h` | An "include guard": stops a header from being pasted in twice |
| `std::` | `std::string`, `std::vector` | Things from the C++ **standard library**, which comes with the compiler |

## 13. Reading compiler errors

When `make` fails, read the **first** error. Later ones are often knock-on effects. The format is `file:line:column: error: message`.

| You see | Usually means |
|---|---|
| `expected ';'` | missing semicolon at the end of the line above |
| `use of undeclared identifier 'x'` / `'x' was not declared` | typo, or a missing `#include` |
| `no member named 'foo' in 'Hand'` | the function doesn't exist, or is spelled differently. Check the `.h` file |
| `undefined reference to ...` / `Undefined symbols` | declared in a `.h` but never written in a `.cpp`, or only one file was compiled |
| `cannot assign to ... const` / `discards qualifiers` | trying to change something inside a `const` function or through a `const&` |
| warning `unused variable` | not an error, but clean it up. This project builds with **zero** warnings |

---

## Answers

- **A:** `11 % 5` is 1, not 0, so `points = 0`.
- **B:** 7 + 6 + 5 + 4 + 3 + 2 + 1 = **28**, the full double-six set.
- **C:** `"0-0"`. The constructor starts at 0-0, and `setPips(4, 9)` refuses because 9 is out of range, so nothing changes.
