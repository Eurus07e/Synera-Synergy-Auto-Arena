# Synera: Synergy Auto-Arena
**Synera: Synergy Auto-Arena** is a lightweight single-player PvE auto-battler game developed with **C++17**, **Qt6**, and **CMake**.
The project explores how an auto-battler game can be structured with object-oriented programming, including a unified unit model, polymorphic hero skills, grid-based combat, BFS pathfinding, equipment systems, synergy effects, and JSON-based save/load support.
## Overview
Synera follows the core loop of an auto-battler game:
1. **Preparation Phase**  
   The player buys units from the shop, manages gold, places units on the board, equips items, and adjusts formation.
2. **Combat Phase**  
   Player units and enemy units automatically search for targets, move across the board, attack, gain mana, and cast skills.
3. **Settlement Phase**  
   The game resolves the result of the round, grants rewards, handles equipment drops, updates player HP, and advances to the next round.
The game is implemented as a single-player PvE experience. Enemy formations are controlled by a simple preparation agent that performs buying, upgrading, deployment, and formation decisions.
## Features
- 8 × 8 grid-based battlefield
- Player board and enemy board areas
- Bench system for undeployed units
- Drag-and-drop unit placement
- Unified `Unit` model for both player and enemy units
- Object-oriented hero system with inheritance and virtual skill dispatch
- 12 hero classes with different skill effects
- Preparation / combat / settlement phase loop
- Unit finite-state machine: `Idle`, `Moving`, `Attacking`, `Casting`, `Dead`
- Automatic target selection
- BFS-based pathfinding with collision avoidance
- Normal attack, mana gain, skill casting, and death handling
- Shop, gold, refresh, purchase, and population systems
- Synergy system with origin and role tags
- Star-upgrade system based on three identical units
- Equipment inventory, equipment wearing, and item synthesis
- Projectile and simple combat visual effects
- JSON save/load system
- Qt Graphics View based GUI
## Tech Stack
- **Language**: C++17
- **GUI Framework**: Qt6
- **Build System**: CMake
- **IDE**: CLion
- **Platform**: macOS
## Project Structure
```text
Synera_Starter/
├── CMakeLists.txt
├── README.md
├── assets/
└── src/
    ├── main.cpp
    ├── core/
    │   ├── game.h / game.cpp
    │   ├── board.h / board.cpp
    │   ├── bench.h / bench.cpp
    │   └── player.h / player.cpp
    ├── entity/
    │   ├── unit/
    │   │   └── unit.h / unit.cpp
    │   └── heros/
    │       ├── cost1/
    │       ├── cost2/
    │       └── cost3/
    └── gui/
        ├── gamewindow.h / gamewindow.cpp
        ├── unititem.h / unititem.cpp
        ├── griditem.h / griditem.cpp
        └── equipmentitem.h / equipmentitem.cpp

Main Modules

* src/main.cpp
    Qt application entry point. It creates the main game window.
* src/core/game.h / src/core/game.cpp
    The central game controller. It manages phases, combat logic, shop logic, economy, equipment, enemy preparation, save/load, and GUI synchronization.
* src/core/board.h / src/core/board.cpp
    The 8 × 8 board data structure. It manages grid occupation, valid positions, and player/enemy board regions.
* src/core/bench.h / src/core/bench.cpp
    The bench system for undeployed units.
* src/core/player.h / src/core/player.cpp
    Player state, including HP, gold, level, population cap, round number, and loss streak.
* src/entity/unit/unit.h / src/entity/unit/unit.cpp
    The base Unit class. It defines common combat attributes and the virtual skill interface.
* src/entity/heros/
    Concrete hero classes. Each hero inherits from Unit and overrides its skill behavior.
* src/gui/
    Qt Graphics View based UI layer, including board cells, draggable units, equipment items, status panels, shop display, and skill/equipment information.

Core Design

Unified Unit Model

Both player units and enemy units use the same Unit class hierarchy. They are not implemented as two separate type systems.

The ownership of a unit is represented by an Owner field:

Owner::PlayerCtrl
Owner::EnemyCtrl

Synergy tags are represented separately through origin and role fields. This keeps combat ownership and synergy calculation independent.

Object-Oriented Hero System

The base class Unit defines common attributes such as:

* HP / Max HP
* Attack damage
* Attack range
* Mana / Max mana
* Attack speed
* Armor and magic resistance
* Star level
* Equipment
* Owner
* Current combat state
* Origin and role tags

It also defines a virtual skill interface:

virtual void castSkill(Game* game, Unit* target);

Concrete hero classes inherit from Unit and override this function to implement different skill effects.

This allows the combat system to call:

caster->castSkill(this, target);

through a base-class pointer. At runtime, C++ dynamic dispatch invokes the correct hero-specific skill implementation.

Hero Classes

The project currently includes 12 hero classes organized by cost:

cost1/
- JarvanIV
- Jhin
- Rumble
- Sona
cost2/
- Ashe
- ChoGath
- XinZhao
- Yasuo
cost3/
- Ahri
- Jinx
- Loris
- Sejuani

Each hero has its own base attributes, synergy tags, and skill behavior.

Combat System

Phase Loop

The game loop is organized around three major phases:

Preparation → Combat → Settlement → Preparation

During preparation, the player can buy units, refresh the shop, equip items, upgrade level, and adjust board placement.

During combat, units are controlled by the combat system. They automatically select targets, move, attack, gain mana, and cast skills.

After one side is defeated, the game enters settlement logic, updates gold and HP, handles item drops, advances the round, and restores the preparation board.

Unit State Machine

Each combat unit is controlled by a finite-state machine:

State	Meaning
Idle	Waiting, stunned, cooling down, or without a valid action
Moving	Moving toward a target
Attacking	Performing a normal attack
Casting	Casting a skill
Dead	Unit has died and will be removed

The combat tick checks unit conditions in priority order:

1. Dead check
2. Stun check
3. Cooldown check
4. Skill casting check
5. Attack range check
6. Movement check

This structure keeps combat behavior predictable and avoids mixing movement, attack, and skill logic in a single uncontrolled flow.

Pathfinding

Synera uses BFS to move units across the grid.

The pathfinding function searches from the unit’s current position toward a reachable cell within attack range of the target. It avoids invalid cells and occupied cells, records parent nodes during search, and moves the unit one step along the recovered path.

Compared with simple greedy movement, BFS handles blocking and basic detours more reliably.

Simplified BFS Logic

1. Start from the unit's current grid position.
2. Push the start cell into a queue.
3. Visit neighboring cells one layer at a time.
4. Ignore out-of-board cells and occupied cells.
5. Record each cell's parent.
6. Stop when a reachable cell within attack range is found.
7. Recover the path through parent records.
8. Move one step along the path.

Target Selection

Units automatically select enemies during combat. The target selection logic considers factors such as:

* Distance to the target
* Current HP ratio
* Attack power
* Whether the unit is ranged or melee
* Whether the enemy is currently attacking this unit

This gives units a basic threat-aware targeting behavior instead of always attacking a purely nearest target.

Economy and Shop

The game includes a basic economy system:

* Initial gold
* Round rewards
* Shop refresh cost
* Unit purchase cost
* Level upgrade cost
* Population limit based on player level
* Interest reward based on current gold
* Loss-streak compensation

The shop randomly offers purchasable heroes from the unit pool. Purchased units are placed into the bench if space is available.

Synergy System

Units have origin and role tags. When multiple units with matching tags are deployed, synergy effects are activated.

Implemented synergy types include both stat-based bonuses and shield-style mechanics, such as:

* HP bonus
* Attack speed bonus
* Armor and magic resistance bonus
* Attack damage bonus
* Shield effects

Synergy effects are applied to combat copies during combat preparation so that the player’s original preparation board state remains stable.

Star Upgrade System

The project implements a three-unit merge system.

When the player owns at least three units with the same name and star level, the game merges them into one higher-star unit. The upgraded unit receives improved attributes such as increased HP and attack damage.

The merge system checks both the board and the bench.

Equipment System

The game includes an equipment inventory and item wearing system.

Units can equip items, and equipment capacity depends on star level. The equipment system supports both basic item stat bonuses and synthesized items.

Example item effects include:

* Attack damage bonus
* Attack speed bonus
* Armor bonus
* Magic resistance bonus
* HP bonus
* Mana-related effects
* Critical rate bonus

The game also supports several item combinations that synthesize stronger equipment with additional passive effects.

Save and Load

The game state can be saved to and loaded from a JSON file.

The save system records information such as:

* Game phase
* Player state
* Enemy state
* Shop slots
* Equipment inventory
* Unit attributes
* Unit positions
* Unit ownership
* Unit star levels
* Unit equipment
* Combat-related temporary states

Loading reconstructs the game state and refreshes the GUI accordingly.

Build and Run

Recommended: CLion

1. Install Qt6.
2. Open the project root directory in CLion.
3. Let CLion load the CMake project.
4. Select the Synera_Starter target.
5. Click Build.
6. Click Run.

Command Line

Command-line build may require manually setting the Qt6 path through CMAKE_PREFIX_PATH.

Example:

cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos
cmake --build build

The exact Qt path depends on the local installation.

How to Play

1. Start the game.
2. Buy units from the shop.
3. Drag units from the bench to the player side of the board.
4. Equip items if available.
5. Upgrade level if enough gold is available.
6. Start combat.
7. Watch units automatically move, attack, gain mana, and cast skills.
8. After combat ends, collect rewards and continue to the next round.
9. Save or load the game when needed.

Screenshots

Screenshots or demo GIFs can be added here.

assets/screenshots/

Suggested screenshots:

* Main game window
* Shop and bench
* Combat scene
* Equipment inventory
* Unit detail panel
* Save/load interface

Current Limitations

* The current visual style is simple and mainly uses basic shapes and text labels.
* Combat balance still needs more systematic testing.
* BFS handles normal blocking cases but may wait in extreme congestion.
* Enemy preparation logic is heuristic-based rather than search-based.
* Audio effects and background music are not included yet.
* The project is primarily developed and tested on macOS with CLion and Qt6.

Development Notes

This project started from a Qt-based starter framework and was extended into a more complete auto-battler prototype. The main development focus was not visual polish, but system design:

* clear object ownership
* maintainable unit and hero hierarchy
* virtual dispatch for skills
* structured combat state transitions
* grid-based movement and collision handling
* serializable game state

Author

Eurus

Undergraduate student at Nanjing University, majoring in Intelligent Science and Technology.

Interested in artificial intelligence, game agents, algorithmic problem solving, and reliable software system design.
