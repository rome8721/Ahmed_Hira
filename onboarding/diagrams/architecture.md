# Architecture

**Short version:** one repository → one C++17 program → one executable (`program`). There are no sub-projects, no shared libraries beyond the C++ standard library, no network calls, and no database. The program touches the outside world in only two ways: **the terminal** (keyboard in, text out) and **text save files** on disk.

## 1. Big picture: layers, deployable, and the outside world

Arrows mean "calls / uses".

```mermaid
flowchart TB
    subgraph OUTSIDE["Outside the program"]
        USER(["Player at the terminal<br/>(stdin / stdout)"])
        FILE[("Save file on disk<br/>plain text, e.g. ./test")]
    end

    subgraph BUILD["Build and tooling (not shipped)"]
        MK["Makefile<br/>make / make run / make clean"]
        VSC[".vscode/tasks.json + launch.json<br/>build task, gdb debug on Windows"]
        CXX["g++, C++17<br/>MinGW on Windows, clang on macOS"]
        MK --> CXX
        VSC --> CXX
    end

    CXX -->|"compiles src/*.cpp"| EXE

    subgraph EXE["Deployable: program.exe on Windows, program on Mac/Linux"]
        MAIN["main()<br/>src/main.cpp"]

        subgraph CONTROL["Control layer"]
            TOUR["Tournament<br/>session loop, resume/save"]
            ROUND["Round<br/>turn loop, draw/pass, scoring"]
        end

        subgraph PLAYERS["Players layer"]
            PLAYER["Player (abstract base)"]
            HUMAN["Human<br/>reads moves"]
            COMP["Computer<br/>strategy: recommend()"]
        end

        subgraph MODEL["Model layer (no I/O here)"]
            LAYOUT["Layout<br/>table + placement rules"]
            BONE["Boneyard<br/>draw pile"]
            HAND["Hand"]
            MOVE["Move (struct)"]
            TILE["Tile"]
        end

        subgraph PERSIST["Persistence"]
            SER["Serializer<br/>all static"]
            GS["GameState / PlayerState<br/>plain records"]
        end

        VIEW["View (namespace)<br/>every print and every read"]

        MAIN --> TOUR
        TOUR --> ROUND
        TOUR --> SER
        ROUND --> PLAYER
        PLAYER -.->|"subclass"| HUMAN
        PLAYER -.->|"subclass"| COMP
        HUMAN -->|"help mode"| COMP
        ROUND --> LAYOUT
        ROUND --> BONE
        ROUND <-->|"toState / restore"| GS
        SER <--> GS
        PLAYER --> HAND
        HAND --> TILE
        BONE --> TILE
        LAYOUT --> TILE
        LAYOUT --> MOVE
        TOUR --> VIEW
        ROUND --> VIEW
        HUMAN --> VIEW
        COMP --> VIEW
    end

    VIEW <-->|"std::cout / std::getline(std::cin)"| USER
    SER <-->|"std::ofstream / std::ifstream"| FILE
```

**What to notice**

- **Only `View` talks to the terminal.** `Tile`, `Hand`, `Boneyard`, `Layout` and `Move` never print or read. That's the author's main design rule.
- **Only `Serializer` opens files.** `Tournament` is the only class that calls `Serializer`.
- **`Round` never knows which player is which.** It holds two `Player*` pointers and calls `chooseMove()`. `Tournament` is the only class that knows a `Human` and a `Computer` exist.
- **`GameState` sits between `Round` and `Serializer`**, so neither needs to know about the other.

## 2. Who owns whom (class diagram)

Filled diamond (`*--`) = "owns / contains". Open diamond (`o--`) = "points to but doesn't own". Triangle (`<|--`) = "inherits from".

```mermaid
classDiagram
    class Tournament {
        -Human m_human
        -Computer m_computer
        -int m_tournamentScore
        +run()
        -askResume(GameState&) bool
        -saveAndQuit(const Round&)
    }
    class Round {
        -Player* m_players
        -int m_next
        -int m_targetScore
        -Layout m_layout
        -Boneyard m_boneyard
        -int m_consecutivePasses
        +startNew()
        +restore(const GameState&) bool
        +play() Outcome
        +toState(int, GameState&)
        -takeTurn(Player*) bool
        -finish()
    }
    class Player {
        <<abstract>>
        #string m_name
        #Hand m_hand
        #int m_roundScore
        #int m_tournamentScore
        +canPlay(const Layout&) bool
        +chooseMove(const Layout&, int)* Move
    }
    class Human {
        +chooseMove(const Layout&, int) Move
        -askEnd(const Layout&, const Tile&) char
    }
    class Computer {
        +chooseMove(const Layout&, int) Move
        +recommend(Hand, Layout, int, string&)$ Move
    }
    class Layout {
        -vector~Tile~ m_line
        -vector~Tile~ m_up
        -vector~Tile~ m_down
        -int m_spinnerIndex
        +canPlace(Tile, char) bool
        +place(Tile, char) bool
        +getOpenEndSum() int
        +toString() string
        +fromString(string) bool
    }
    class Boneyard {
        -vector~Tile~ m_tiles
        +reset()
        +shuffle()
        +drawTile(Tile&) bool
        +addToBottom(Tile) bool
    }
    class Hand {
        -vector~Tile~ m_tiles
        +addTile(Tile) bool
        +removeTile(Tile) bool
        +contains(Tile) bool
        +getSum() int
    }
    class Tile {
        -int m_leftPips
        -int m_rightPips
        +isDouble() bool
        +flipped() Tile
        +fromString(string, Tile&)$ bool
    }
    class Move {
        <<struct>>
        +Tile tile
        +char end
        +bool valid
    }

    Tournament *-- Human
    Tournament *-- Computer
    Tournament ..> Round : creates one per round
    Round o-- Player : 2 pointers
    Round *-- Layout
    Round *-- Boneyard
    Player <|-- Human
    Player <|-- Computer
    Player *-- Hand
    Hand *-- Tile
    Boneyard *-- Tile
    Layout *-- Tile
    Move *-- Tile
    Human ..> Computer : help calls recommend()
```

## 3. Header dependencies (who `#include`s whom)

Useful when you add a file or get a compile error about a missing type.

```mermaid
flowchart LR
    main_cpp["main.cpp"] --> tournament_h["tournament.h"]
    tournament_h --> human_h["human.h"] & computer_h["computer.h"] & gamestate_h["gamestate.h"]
    tournament_cpp["tournament.cpp"] --> round_h["round.h"] & serializer_h["serializer.h"] & view_h["view.h"]
    round_h --> player_h["player.h"] & layout_h["layout.h"] & boneyard_h["boneyard.h"] & gamestate_h
    human_h --> player_h
    computer_h --> player_h
    human_cpp["human.cpp"] --> computer_h & view_h
    computer_cpp["computer.cpp"] --> view_h
    player_h --> hand_h["hand.h"] & layout_h & move_h["move.h"]
    serializer_h --> gamestate_h
    gamestate_h --> hand_h
    view_h --> hand_h & layout_h
    layout_h --> tile_h["tile.h"] & move_h
    move_h --> tile_h
    hand_h --> tile_h
    boneyard_h --> tile_h
```

## 4. External services and libraries

| Kind | What's used |
|---|---|
| Third-party libraries | None |
| Standard library | `<iostream>`, `<fstream>`, `<sstream>`, `<string>`, `<vector>`, `<map>`, `<algorithm>`, `<random>`, `<cctype>` |
| Network / APIs / DB | None |
| Randomness | `std::random_device` seeds `std::mt19937` in `Boneyard::shuffle()`, so every new game is different. A resumed game has no randomness until the next round starts |
| Files | Save files, at whatever path the user types, relative to the working directory |
