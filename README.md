# Synera: Synergy Auto-Arena
<small>（Synera：协同自走棋竞技场）</small>

**Synera: Synergy Auto-Arena** is a lightweight single-player PvE auto-battler game developed with **C++17**, **Qt6**, and **CMake**.  
<small>（**Synera: Synergy Auto-Arena** 是一款使用 **C++17**、**Qt6** 和 **CMake** 开发的轻量级单人 PvE 自走棋游戏。）</small>

The project explores how an auto-battler game can be structured with object-oriented programming, including a unified unit model, polymorphic hero skills, grid-based combat, BFS pathfinding, equipment systems, synergy effects, and JSON-based save/load support.  
<small>（本项目探索如何用面向对象程序设计组织一个自走棋游戏系统，包括统一单位模型、多态英雄技能、网格战斗、BFS 寻路、装备系统、羁绊效果，以及基于 JSON 的存档与读档支持。）</small>

## Overview
<small>（项目概述）</small>

Synera follows the core loop of an auto-battler game:  
<small>（Synera 遵循自走棋游戏的核心循环：）</small>

1. **Preparation Phase**  
   <small>（**准备阶段**）</small>

   The player buys units from the shop, manages gold, places units on the board, equips items, and adjusts formation.  
   <small>（玩家可以从商店购买单位，管理金币，将单位放置到棋盘上，装备物品，并调整阵型。）</small>

2. **Combat Phase**  
   <small>（**战斗阶段**）</small>

   Player units and enemy units automatically search for targets, move across the board, attack, gain mana, and cast skills.  
   <small>（玩家单位和敌方单位会自动寻找目标，在棋盘上移动、攻击、获得法力值并释放技能。）</small>

3. **Settlement Phase**  
   <small>（**结算阶段**）</small>

   The game resolves the result of the round, grants rewards, handles equipment drops, updates player HP, and advances to the next round.  
   <small>（游戏会结算本回合结果，发放奖励，处理装备掉落，更新玩家生命值，并进入下一回合。）</small>

The game is implemented as a single-player PvE experience. Enemy formations are controlled by a simple preparation agent that performs buying, upgrading, deployment, and formation decisions.  
<small>（本游戏实现为单人 PvE 体验。敌方阵容由一个简单的准备阶段智能体控制，用于执行购买、升级、部署和阵型决策。）</small>

## Features
<small>（功能特性）</small>

- 8 × 8 grid-based battlefield  
  <small>（8 × 8 网格战场）</small>

- Player board and enemy board areas  
  <small>（玩家棋盘区域与敌方棋盘区域）</small>

- Bench system for undeployed units  
  <small>（用于存放未上场单位的备战区系统）</small>

- Drag-and-drop unit placement  
  <small>（支持拖拽式单位放置）</small>

- Unified `Unit` model for both player and enemy units  
  <small>（玩家单位和敌方单位共用统一的 `Unit` 模型）</small>

- Object-oriented hero system with inheritance and virtual skill dispatch  
  <small>（基于继承和虚函数技能分派的面向对象英雄系统）</small>

- 12 hero classes with different skill effects  
  <small>（12 个具有不同技能效果的英雄类）</small>

- Preparation / combat / settlement phase loop  
  <small>（准备 / 战斗 / 结算阶段循环）</small>

- Unit finite-state machine: `Idle`, `Moving`, `Attacking`, `Casting`, `Dead`  
  <small>（单位有限状态机：`Idle`、`Moving`、`Attacking`、`Casting`、`Dead`）</small>

- Automatic target selection  
  <small>（自动目标选择）</small>

- BFS-based pathfinding with collision avoidance  
  <small>（基于 BFS 的寻路，并支持碰撞规避）</small>

- Normal attack, mana gain, skill casting, and death handling  
  <small>（普通攻击、法力值获取、技能释放和死亡处理）</small>

- Shop, gold, refresh, purchase, and population systems  
  <small>（商店、金币、刷新、购买和人口系统）</small>

- Synergy system with origin and role tags  
  <small>（基于起源标签和职业标签的羁绊系统）</small>

- Star-upgrade system based on three identical units  
  <small>（基于三个相同单位合成的升星系统）</small>

- Equipment inventory, equipment wearing, and item synthesis  
  <small>（装备背包、装备穿戴和装备合成）</small>

- Projectile and simple combat visual effects  
  <small>（弹道效果和简单战斗视觉效果）</small>

- JSON save/load system  
  <small>（JSON 存档与读档系统）</small>

- Qt Graphics View based GUI  
  <small>（基于 Qt Graphics View 的图形界面）</small>

## Tech Stack
<small>（技术栈）</small>

- **Language**: C++17  
  <small>（**编程语言**：C++17）</small>

- **GUI Framework**: Qt6  
  <small>（**图形界面框架**：Qt6）</small>

- **Build System**: CMake  
  <small>（**构建系统**：CMake）</small>

- **IDE**: CLion  
  <small>（**开发环境**：CLion）</small>

- **Platform**: macOS  
  <small>（**平台**：macOS）</small>

## Project Structure
<small>（项目结构）</small>

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
```

## Main Modules
<small>（主要模块）</small>

- `src/main.cpp`  
  <small>（`src/main.cpp`）</small>

  Qt application entry point. It creates the main game window.  
  <small>（Qt 应用程序入口，用于创建主游戏窗口。）</small>

- `src/core/game.h / src/core/game.cpp`  
  <small>（`src/core/game.h / src/core/game.cpp`）</small>

  The central game controller. It manages phases, combat logic, shop logic, economy, equipment, enemy preparation, save/load, and GUI synchronization.  
  <small>（核心游戏控制器，负责管理阶段切换、战斗逻辑、商店逻辑、经济系统、装备系统、敌方准备、存档读档和 GUI 同步。）</small>

- `src/core/board.h / src/core/board.cpp`  
  <small>（`src/core/board.h / src/core/board.cpp`）</small>

  The 8 × 8 board data structure. It manages grid occupation, valid positions, and player/enemy board regions.  
  <small>（8 × 8 棋盘数据结构，负责管理格子占用、合法位置以及玩家 / 敌方棋盘区域。）</small>

- `src/core/bench.h / src/core/bench.cpp`  
  <small>（`src/core/bench.h / src/core/bench.cpp`）</small>

  The bench system for undeployed units.  
  <small>（用于管理未上场单位的备战区系统。）</small>

- `src/core/player.h / src/core/player.cpp`  
  <small>（`src/core/player.h / src/core/player.cpp`）</small>

  Player state, including HP, gold, level, population cap, round number, and loss streak.  
  <small>（玩家状态，包括生命值、金币、等级、人口上限、回合数和连败次数。）</small>

- `src/entity/unit/unit.h / src/entity/unit/unit.cpp`  
  <small>（`src/entity/unit/unit.h / src/entity/unit/unit.cpp`）</small>

  The base Unit class. It defines common combat attributes and the virtual skill interface.  
  <small>（基础 Unit 类，定义通用战斗属性和虚函数技能接口。）</small>

- `src/entity/heros/`  
  <small>（`src/entity/heros/`）</small>

  Concrete hero classes. Each hero inherits from Unit and overrides its skill behavior.  
  <small>（具体英雄类，每个英雄都继承自 Unit，并重写自己的技能行为。）</small>

- `src/gui/`  
  <small>（`src/gui/`）</small>

  Qt Graphics View based UI layer, including board cells, draggable units, equipment items, status panels, shop display, and skill/equipment information.  
  <small>（基于 Qt Graphics View 的 UI 层，包括棋盘格、可拖拽单位、装备物品、状态面板、商店显示以及技能 / 装备信息。）</small>

## Core Design
<small>（核心设计）</small>

### Unified Unit Model
<small>（统一单位模型）</small>

Both player units and enemy units use the same Unit class hierarchy. They are not implemented as two separate type systems.  
<small>（玩家单位和敌方单位使用同一套 Unit 类层次结构，而不是被实现为两套独立的类型系统。）</small>

The ownership of a unit is represented by an Owner field:  
<small>（单位的归属由 Owner 字段表示：）</small>

```cpp
Owner::PlayerCtrl
Owner::EnemyCtrl
```

Synergy tags are represented separately through origin and role fields. This keeps combat ownership and synergy calculation independent.  
<small>（羁绊标签通过 origin 和 role 字段单独表示，从而使战斗归属和羁绊计算彼此独立。）</small>

### Object-Oriented Hero System
<small>（面向对象英雄系统）</small>

The base class Unit defines common attributes such as:  
<small>（基类 Unit 定义了通用属性，例如：）</small>

- HP / Max HP  
  <small>（生命值 / 最大生命值）</small>

- Attack damage  
  <small>（攻击力）</small>

- Attack range  
  <small>（攻击范围）</small>

- Mana / Max mana  
  <small>（法力值 / 最大法力值）</small>

- Attack speed  
  <small>（攻击速度）</small>

- Armor and magic resistance  
  <small>（护甲和魔法抗性）</small>

- Star level  
  <small>（星级）</small>

- Equipment  
  <small>（装备）</small>

- Owner  
  <small>（单位归属）</small>

- Current combat state  
  <small>（当前战斗状态）</small>

- Origin and role tags  
  <small>（起源标签和职业标签）</small>

It also defines a virtual skill interface:  
<small>（它还定义了一个虚函数技能接口：）</small>

```cpp
virtual void castSkill(Game* game, Unit* target);
```

Concrete hero classes inherit from Unit and override this function to implement different skill effects.  
<small>（具体英雄类继承自 Unit，并重写该函数以实现不同的技能效果。）</small>

This allows the combat system to call:  
<small>（这使得战斗系统可以调用：）</small>

```cpp
caster->castSkill(this, target);
```

through a base-class pointer. At runtime, C++ dynamic dispatch invokes the correct hero-specific skill implementation.  
<small>（通过基类指针调用技能。在运行时，C++ 的动态绑定机制会调用对应英雄自己的技能实现。）</small>

## Hero Classes
<small>（英雄类）</small>

The project currently includes 12 hero classes organized by cost:  
<small>（本项目目前包含 12 个英雄类，并按照费用进行组织：）</small>

`cost1/`  
<small>（1 费英雄）</small>

- JarvanIV  
  <small>（嘉文四世）</small>

- Jhin  
  <small>（烬）</small>

- Rumble  
  <small>（兰博）</small>

- Sona  
  <small>（娑娜）</small>

`cost2/`  
<small>（2 费英雄）</small>

- Ashe  
  <small>（艾希）</small>

- ChoGath  
  <small>（科加斯）</small>

- XinZhao  
  <small>（赵信）</small>

- Yasuo  
  <small>（亚索）</small>

`cost3/`  
<small>（3 费英雄）</small>

- Ahri  
  <small>（阿狸）</small>

- Jinx  
  <small>（金克丝）</small>

- Loris  
  <small>（洛里斯）</small>

- Sejuani  
  <small>（瑟庄妮）</small>

Each hero has its own base attributes, synergy tags, and skill behavior.  
<small>（每个英雄都有自己的基础属性、羁绊标签和技能行为。）</small>

## Combat System
<small>（战斗系统）</small>

### Phase Loop
<small>（阶段循环）</small>

The game loop is organized around three major phases:  
<small>（游戏循环围绕三个主要阶段组织：）</small>

```text
Preparation → Combat → Settlement → Preparation
```

During preparation, the player can buy units, refresh the shop, equip items, upgrade level, and adjust board placement.  
<small>（在准备阶段，玩家可以购买单位、刷新商店、装备物品、升级等级，并调整棋盘站位。）</small>

During combat, units are controlled by the combat system. They automatically select targets, move, attack, gain mana, and cast skills.  
<small>（在战斗阶段，单位由战斗系统控制，会自动选择目标、移动、攻击、获得法力值并释放技能。）</small>

After one side is defeated, the game enters settlement logic, updates gold and HP, handles item drops, advances the round, and restores the preparation board.  
<small>（当一方被击败后，游戏进入结算逻辑，更新金币和生命值，处理物品掉落，推进回合，并恢复准备阶段棋盘。）</small>

### Unit State Machine
<small>（单位状态机）</small>

Each combat unit is controlled by a finite-state machine:  
<small>（每个战斗单位都由一个有限状态机控制：）</small>

| State | Meaning |
|---|---|
| Idle | Waiting, stunned, cooling down, or without a valid action |
| Moving | Moving toward a target |
| Attacking | Performing a normal attack |
| Casting | Casting a skill |
| Dead | Unit has died and will be removed |

<small>（状态含义：Idle 表示等待、眩晕、冷却中或没有合法行动；Moving 表示向目标移动；Attacking 表示进行普通攻击；Casting 表示释放技能；Dead 表示单位已经死亡并将被移除。）</small>

The combat tick checks unit conditions in priority order:  
<small>（战斗 tick 会按照优先级顺序检查单位状态：）</small>

1. Dead check  
   <small>（死亡检查）</small>

2. Stun check  
   <small>（眩晕检查）</small>

3. Cooldown check  
   <small>（冷却检查）</small>

4. Skill casting check  
   <small>（技能释放检查）</small>

5. Attack range check  
   <small>（攻击范围检查）</small>

6. Movement check  
   <small>（移动检查）</small>

This structure keeps combat behavior predictable and avoids mixing movement, attack, and skill logic in a single uncontrolled flow.  
<small>（这种结构使战斗行为更加可预测，并避免将移动、攻击和技能逻辑混杂在一个不可控流程中。）</small>

## Pathfinding
<small>（寻路系统）</small>

Synera uses BFS to move units across the grid.  
<small>（Synera 使用 BFS 在网格上移动单位。）</small>

The pathfinding function searches from the unit’s current position toward a reachable cell within attack range of the target. It avoids invalid cells and occupied cells, records parent nodes during search, and moves the unit one step along the recovered path.  
<small>（寻路函数从单位当前位置开始搜索，寻找一个位于目标攻击范围内的可达格子。它会避开非法格子和被占用格子，在搜索过程中记录父节点，并让单位沿恢复出的路径移动一步。）</small>

Compared with simple greedy movement, BFS handles blocking and basic detours more reliably.  
<small>（相比简单贪心移动，BFS 能更可靠地处理阻挡和基础绕路情况。）</small>

### Simplified BFS Logic
<small>（简化 BFS 逻辑）</small>

1. Start from the unit's current grid position.  
   <small>（从单位当前所在的网格位置开始。）</small>

2. Push the start cell into a queue.  
   <small>（将起点格子加入队列。）</small>

3. Visit neighboring cells one layer at a time.  
   <small>（逐层访问相邻格子。）</small>

4. Ignore out-of-board cells and occupied cells.  
   <small>（忽略棋盘外格子和已被占用的格子。）</small>

5. Record each cell's parent.  
   <small>（记录每个格子的父节点。）</small>

6. Stop when a reachable cell within attack range is found.  
   <small>（当找到一个处于攻击范围内的可达格子时停止搜索。）</small>

7. Recover the path through parent records.  
   <small>（通过父节点记录恢复路径。）</small>

8. Move one step along the path.  
   <small>（沿路径移动一步。）</small>

## Target Selection
<small>（目标选择）</small>

Units automatically select enemies during combat. The target selection logic considers factors such as:  
<small>（单位会在战斗中自动选择敌人。目标选择逻辑会考虑以下因素：）</small>

- Distance to the target  
  <small>（到目标的距离）</small>

- Current HP ratio  
  <small>（当前生命值比例）</small>

- Attack power  
  <small>（攻击能力）</small>

- Whether the unit is ranged or melee  
  <small>（该单位是远程单位还是近战单位）</small>

- Whether the enemy is currently attacking this unit  
  <small>（敌人当前是否正在攻击该单位）</small>

This gives units a basic threat-aware targeting behavior instead of always attacking a purely nearest target.  
<small>（这使单位具备基础的威胁感知目标选择行为，而不是永远只攻击最近目标。）</small>

## Economy and Shop
<small>（经济与商店系统）</small>

The game includes a basic economy system:  
<small>（游戏包含一个基础经济系统：）</small>

- Initial gold  
  <small>（初始金币）</small>

- Round rewards  
  <small>（回合奖励）</small>

- Shop refresh cost  
  <small>（商店刷新费用）</small>

- Unit purchase cost  
  <small>（单位购买费用）</small>

- Level upgrade cost  
  <small>（等级升级费用）</small>

- Population limit based on player level  
  <small>（基于玩家等级的人口上限）</small>

- Interest reward based on current gold  
  <small>（基于当前金币数量的利息奖励）</small>

- Loss-streak compensation  
  <small>（连败补偿）</small>

The shop randomly offers purchasable heroes from the unit pool. Purchased units are placed into the bench if space is available.  
<small>（商店会从单位池中随机提供可购买英雄。如果备战区有空位，购买的单位会被放入备战区。）</small>

## Synergy System
<small>（羁绊系统）</small>

Units have origin and role tags. When multiple units with matching tags are deployed, synergy effects are activated.  
<small>（单位拥有起源标签和职业标签。当多个拥有相同标签的单位被部署时，对应羁绊效果会被激活。）</small>

Implemented synergy types include both stat-based bonuses and shield-style mechanics, such as:  
<small>（已实现的羁绊类型包括属性加成和护盾类机制，例如：）</small>

- HP bonus  
  <small>（生命值加成）</small>

- Attack speed bonus  
  <small>（攻击速度加成）</small>

- Armor and magic resistance bonus  
  <small>（护甲与魔法抗性加成）</small>

- Attack damage bonus  
  <small>（攻击力加成）</small>

- Shield effects  
  <small>（护盾效果）</small>

Synergy effects are applied to combat copies during combat preparation so that the player’s original preparation board state remains stable.  
<small>（羁绊效果会在战斗准备阶段应用到战斗副本上，从而保持玩家原始准备棋盘状态稳定不变。）</small>

## Star Upgrade System
<small>（升星系统）</small>

The project implements a three-unit merge system.  
<small>（本项目实现了三合一升星系统。）</small>

When the player owns at least three units with the same name and star level, the game merges them into one higher-star unit. The upgraded unit receives improved attributes such as increased HP and attack damage.  
<small>（当玩家拥有至少三个名称和星级都相同的单位时，游戏会将它们合成为一个更高星级单位。升星后的单位会获得更高属性，例如更多生命值和攻击力。）</small>

The merge system checks both the board and the bench.  
<small>（合成系统会同时检查棋盘和备战区。）</small>

## Equipment System
<small>（装备系统）</small>

The game includes an equipment inventory and item wearing system.  
<small>（游戏包含装备背包和装备穿戴系统。）</small>

Units can equip items, and equipment capacity depends on star level. The equipment system supports both basic item stat bonuses and synthesized items.  
<small>（单位可以穿戴装备，装备容量取决于星级。装备系统同时支持基础装备属性加成和合成装备。）</small>

Example item effects include:  
<small>（示例装备效果包括：）</small>

- Attack damage bonus  
  <small>（攻击力加成）</small>

- Attack speed bonus  
  <small>（攻击速度加成）</small>

- Armor bonus  
  <small>（护甲加成）</small>

- Magic resistance bonus  
  <small>（魔法抗性加成）</small>

- HP bonus  
  <small>（生命值加成）</small>

- Mana-related effects  
  <small>（法力值相关效果）</small>

- Critical rate bonus  
  <small>（暴击率加成）</small>

The game also supports several item combinations that synthesize stronger equipment with additional passive effects.  
<small>（游戏还支持若干装备组合，可以合成带有额外被动效果的更强装备。）</small>

## Save and Load
<small>（存档与读档）</small>

The game state can be saved to and loaded from a JSON file.  
<small>（游戏状态可以保存到 JSON 文件，也可以从 JSON 文件读取。）</small>

The save system records information such as:  
<small>（存档系统会记录以下信息：）</small>

- Game phase  
  <small>（游戏阶段）</small>

- Player state  
  <small>（玩家状态）</small>

- Enemy state  
  <small>（敌方状态）</small>

- Shop slots  
  <small>（商店槽位）</small>

- Equipment inventory  
  <small>（装备背包）</small>

- Unit attributes  
  <small>（单位属性）</small>

- Unit positions  
  <small>（单位位置）</small>

- Unit ownership  
  <small>（单位归属）</small>

- Unit star levels  
  <small>（单位星级）</small>

- Unit equipment  
  <small>（单位装备）</small>

- Combat-related temporary states  
  <small>（战斗相关临时状态）</small>

Loading reconstructs the game state and refreshes the GUI accordingly.  
<small>（读档会重建游戏状态，并相应刷新图形界面。）</small>

## Build and Run
<small>（构建与运行）</small>

### Recommended: CLion
<small>（推荐方式：CLion）</small>

1. Install Qt6.  
   <small>（安装 Qt6。）</small>

2. Open the project root directory in CLion.  
   <small>（在 CLion 中打开项目根目录。）</small>

3. Let CLion load the CMake project.  
   <small>（让 CLion 加载 CMake 项目。）</small>

4. Select the Synera_Starter target.  
   <small>（选择 Synera_Starter 构建目标。）</small>

5. Click Build.  
   <small>（点击 Build 进行构建。）</small>

6. Click Run.  
   <small>（点击 Run 运行程序。）</small>

### Command Line
<small>（命令行方式）</small>

Command-line build may require manually setting the Qt6 path through CMAKE_PREFIX_PATH.  
<small>（命令行构建可能需要通过 CMAKE_PREFIX_PATH 手动设置 Qt6 路径。）</small>

Example:  
<small>（示例：）</small>

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos
cmake --build build
```

The exact Qt path depends on the local installation.  
<small>（具体 Qt 路径取决于本地安装位置。）</small>

## How to Play
<small>（玩法说明）</small>

1. Start the game.  
   <small>（启动游戏。）</small>

2. Buy units from the shop.  
   <small>（从商店购买单位。）</small>

3. Drag units from the bench to the player side of the board.  
   <small>（将单位从备战区拖拽到玩家侧棋盘。）</small>

4. Equip items if available.  
   <small>（如果有装备，可以给单位穿戴装备。）</small>

5. Upgrade level if enough gold is available.  
   <small>（如果金币足够，可以升级等级。）</small>

6. Start combat.  
   <small>（开始战斗。）</small>

7. Watch units automatically move, attack, gain mana, and cast skills.  
   <small>（观察单位自动移动、攻击、获得法力值并释放技能。）</small>

8. After combat ends, collect rewards and continue to the next round.  
   <small>（战斗结束后领取奖励，并进入下一回合。）</small>

9. Save or load the game when needed.  
   <small>（需要时可以保存或读取游戏。）</small>

## Screenshots
<small>（截图）</small>

Screenshots or demo GIFs can be added here.  
<small>（可以在这里添加截图或演示 GIF。）</small>

```text
assets/screenshots/
```

Suggested screenshots:  
<small>（建议添加的截图：）</small>

- Main game window  
  <small>（主游戏窗口）</small>

- Shop and bench  
  <small>（商店和备战区）</small>

- Combat scene  
  <small>（战斗场景）</small>

- Equipment inventory  
  <small>（装备背包）</small>

- Unit detail panel  
  <small>（单位详情面板）</small>

- Save/load interface  
  <small>（存档 / 读档界面）</small>

## Current Limitations
<small>（当前局限）</small>

- The current visual style is simple and mainly uses basic shapes and text labels.  
  <small>（当前视觉风格较为简单，主要使用基础图形和文字标签。）</small>

- Combat balance still needs more systematic testing.  
  <small>（战斗平衡性仍需要更系统的测试。）</small>

- BFS handles normal blocking cases but may wait in extreme congestion.  
  <small>（BFS 可以处理普通阻挡情况，但在极端拥堵时可能会出现等待。）</small>

- Enemy preparation logic is heuristic-based rather than search-based.  
  <small>（敌方准备逻辑基于启发式规则，而不是基于搜索算法。）</small>

- Audio effects and background music are not included yet.  
  <small>（目前尚未加入音效和背景音乐。）</small>

- The project is primarily developed and tested on macOS with CLion and Qt6.  
  <small>（本项目主要在 macOS、CLion 和 Qt6 环境下开发与测试。）</small>

## Development Notes
<small>（开发说明）</small>

This project started from a Qt-based starter framework and was extended into a more complete auto-battler prototype. The main development focus was not visual polish, but system design:  
<small>（本项目从一个基于 Qt 的初始框架出发，扩展为一个较完整的自走棋原型。主要开发重点不是视觉美化，而是系统设计：）</small>

- clear object ownership  
  <small>（清晰的对象所有权）</small>

- maintainable unit and hero hierarchy  
  <small>（可维护的单位与英雄类层次结构）</small>

- virtual dispatch for skills  
  <small>（技能的虚函数动态分派）</small>

- structured combat state transitions  
  <small>（结构化的战斗状态转换）</small>

- grid-based movement and collision handling  
  <small>（基于网格的移动与碰撞处理）</small>

- serializable game state  
  <small>（可序列化的游戏状态）</small>

## Author
<small>（作者）</small>

Eurus  
<small>（欧若斯）</small>

Undergraduate student at Nanjing University, majoring in Intelligent Science and Technology.  
<small>（南京大学智能科学与技术专业本科生。）</small>

Interested in artificial intelligence, game agents, algorithmic problem solving, and reliable software system design.  
<small>（兴趣方向包括人工智能、游戏智能体、算法问题求解和可靠软件系统设计。）</small>
