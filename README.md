# Synera: Synergy Auto-Arena

## 1. 基本信息

- **项目名称**：Synera: Synergy Auto-Arena
- **课程**：高级程序设计 PA
- **姓名**：束宇轩
- **学号**：251880244
- **开发语言**：C++17
- **图形界面**：Qt6
- **构建工具**：CMake
- **开发环境**：macOS + CLion + Qt6
- **项目类型**：单机 PvE 轻量级自走棋

本项目基于课程提供的 Starter Code 继续开发，在原 Demo 框架（网格、拖拽、Qt 基础场景）基础上，重构并扩展了完整的面向对象战斗单位体系、经济系统、羁绊系统、升星机制、装备系统、存档读档、敌方 AI 运营 agent、战斗弹道与特效等模块。核心逻辑全部自研。

### 各阶段完成度总览

| 阶段 | 完成状态 | 实现功能                                    |
|------|----------|-----------------------------------------|
| 阶段一（棋盘/单位/拖拽/GUI） | 已完成 | 8×8 棋盘、备战区、单位基类、owner 区分、拖拽换位/回弹、GUI 展示 |
| 阶段二（战斗/状态机/寻路/技能） | 已完成 | 三阶段循环、FSM、BFS 寻路、索敌、12 英雄多态技能、胜负结算      |
| 阶段三（商店/经济/羁绊/升星/装备/存档） | 已完成* | 全部实现，其中，商店槽位由 5 改为 2 ，为平衡性调整            |
| 阶段四（扩展） | 已完成 | 利息机制、连败补偿、装备合成树、弹道特效、智能索敌               |

## 2. 文件结构

```text
Synera_Starter/
├── CMakeLists.txt                  # CMake 构建配置，依赖 Qt6 Widgets/Core/Gui
├── README.md                       # 项目说明与验收文档（本文件）
├── assets/                         # 美术资源（当前使用简单方块+文字标注）
└── src/
    ├── main.cpp                    # Qt QApplication 入口，创建并显示 GameWindow
    ├── core/                       # 核心逻辑层
    │   ├── game.h / game.cpp       # 游戏主控制器：场景、阶段、战斗、经济、装备、存档、敌方 agent
    │   ├── board.h / board.cpp     # 8×8 棋盘数据结构，管理地块占用与玩家/敌方半场判断
    │   ├── bench.h / bench.cpp     # 一维备战区（8 格），存放未上阵单位
    │   └── player.h / player.cpp   # 玩家数据：血量、金币、等级、人口上限、轮次
    ├── entity/                     # 实体层
    │   ├── unit/
    │   │   └── unit.h / unit.cpp   # Unit 基类：所有战斗单位的统一模型
    │   └── heros/                  # 英雄派生类（12 名）
    │       ├── cost1/              # 1 费：JarvanIV, Jhin, Rumble, Sona
    │       ├── cost2/              # 2 费：Ashe, ChoGath, XinZhao, Yasuo
    │       └── cost3/              # 3 费：Ahri, Jinx, Loris, Sejuani
    └── gui/                        # GUI 展示层
        ├── gamewindow.h / .cpp     # 主窗口：棋盘、状态栏、商店、装备栏、羁绊展示
        ├── unititem.h / .cpp       # 单位图形项：拖拽、高亮、右键详情卡
        ├── griditem.h / .cpp       # 棋盘格子图形项（六边形）
        └── equipmentitem.h / .cpp  # 可拖拽的装备图形项
```

- `src/main.cpp`：Qt 应用入口，创建 QApplication 和 GameWindow 主窗口。
- `src/core/`：游戏核心逻辑层。`game.cpp` 约 3071 行，是项目最核心文件，负责整合所有子系统。
- `src/entity/unit/`：`Unit` 基类定义在 `unit.h`，约 186 行，所有 12 个英雄类均继承自它。
- `src/entity/heros/`：按费用（cost1/cost2/cost3）组织，每个英雄重写 `castSkill()` 实现技能多态。
- `src/gui/`：基于 Qt Graphics View 框架的 UI 层。`gamewindow.cpp` 约 1226 行。

## 3. 核心类与数据结构设计

### 3.1 Game 类（`src/core/game.h` / `src/core/game.cpp`）

Game 是游戏主控制器（继承 QObject），负责协调所有子系统：

- **阶段管理**：维护 `GamePhase`（Preparation / Combat），控制 `startCombat()`、`endCombat()`、`finishCombat()` 流程。
- **单位管理**：维护 `m_units`（所有单位列表）、`m_combatUnits`（战斗副本单位）、`m_preCombatBoardUnits`（备战阵容快照）。
- **棋盘/备战区**：持有 `Board m_board`、`Bench m_bench`、`Bench m_enemyBench` 实例。
- **商店**：`m_shopSlots` 存储当前商店槽位（`std::vector<ShopSlot>`），`rollShop()` 刷新，`buyShopUnit()` 购买。
- **战斗循环**：`combatTick()` 每 200ms 触发一次，遍历双方存活单位执行 FSM 逻辑。
- **敌方 AI**：`runEnemyPrepareAgent()` 模拟敌方经营决策（购买、刷新、出售、升级、布阵）。
- **装备**：`m_equipmentInventory` 管理装备库存，`equipInventoryItem()` 处理穿戴与合成。
- **存档**：`saveGame()` / `loadGame()` 将完整游戏状态序列化为 JSON 文件。
- **GUI 信号**：通过 `stateChanged()`、`gameFinished()` 信号驱动 GUI 更新。

### 3.2 Player 类（`src/core/player.h` / `src/core/player.cpp`）

Player 管理单个玩家的经营数据：

- `m_hp`：血量，初始 100（`kInitialHp = 100`），归零则游戏失败。
- `m_gold`：金币，初始 6（`kInitialGold = 6`），用于购买、刷新、升级。
- `m_level`：等级 = 人口上限。初始 1，最高 10（`kMaxLevel = 10`）。
- `m_levelProgress`：升级进度，每次购买进度消耗 4 金币获得 4 点，各等级所需进度为 {4, 4, 6, 10, 15, 30, 36, 60, 70}。
- `m_round`：当前轮次，`advanceRound()` 推进。
- `m_lossStreak`：连败计数，`recordWin()` 清零，`recordLoss()` 递增并返回当前值。
- `unitCap()`：返回 `m_level`，即当前可上阵的最大单位数。

### 3.3 Board 类（`src/core/board.h` / `src/core/board.cpp`）

Board 管理 8×8 网格上的地块占用：

- 8 行 × 8 列（`ROWS=8, COLS=8`）。
- `addUnit()` / `removeUnit()` 控制单位摆放到指定坐标。
- `getUnitAt(pos)` / `hasUnitAt(pos)` 查询地块占用。
- `isPlayerHalf(pos)`：判断坐标是否在玩家半场（第 4-7 行，0-based）。
- `isValidPosition(pos)`：判断坐标是否在棋盘范围内。

### 3.4 Bench 类（`src/core/bench.h` / `src/core/bench.cpp`）

Bench 管理一维备战区：

- `SLOTS = 8`：固定 8 个槽位。
- `addUnit()` / `removeUnit()` / `getUnitAt()` / `hasUnitAt()` 管理槽位。
- `swapUnits()`：交换两个槽位的单位。
- `findUnit()`：查找单位所在的槽位索引。

### 3.5 Unit 基类（`src/entity/unit/unit.h` / `src/entity/unit/unit.cpp`）

Unit 是所有战斗单位的统一基类。我方和敌方单位都是 Unit 实例，通过 `m_owner` 字段区分归属（`Owner::PlayerCtrl` / `Owner::EnemyCtrl`），不建两套类型体系。`m_origins` 和 `m_roles` 仅用于羁绊计算，不用于敌我区分。

**核心属性**：

| 字段 | 类型 | 说明 |
|------|------|------|
| `m_hp` / `m_maxHp` | int | 当前/最大生命值 |
| `m_atk` | int | 攻击力 |
| `m_range` | int | 攻击距离（格数） |
| `m_maxMana` / `m_mana` | int | 最大/当前法力值 |
| `m_attackSpeed` | double | 攻击速度倍率 |
| `m_armor` / `m_magicResist` | int | 护甲 / 魔抗 |
| `m_abilityPower` | int | 法术强度 |
| `m_critRate` | double | 暴击率 |
| `m_star` | int | 星级（1-3） |
| `m_cost` | int | 购买费用 |
| `m_owner` | Owner | 控制归属（PlayerCtrl / EnemyCtrl） |
| `m_state` | UnitState | 当前状态机状态 |
| `m_origins` / `m_roles` | vector | 羁绊标签（种族/职业） |
| `m_equipment` | vector | 已穿戴的装备类型列表 |
| `m_heroType` | int | 英雄类型枚举值，用于存档恢复 |
| `m_positionType` | UnitPositionType | Frontline / Backline |

**虚函数接口**：

- `virtual void castSkill(Game* game, Unit* target)`：英雄技能的多态入口。基类默认空实现，各派生类重写以执行不同技能效果。

### 3.6 英雄派生类（`src/entity/heros/`）

共 12 名英雄，按费用分为 3 档，均继承自 Unit 并重写 `castSkill()`：

**1 费英雄（cost1/）**：

| 英雄 | 羁绊 | 技能 |
|------|------|------|
| Jarvan IV | 德玛西亚 + 护卫 | 德邦军旗：获得持续 4 秒护盾 + 全场友军攻速提升 4 秒 |
| Jhin | 艾欧尼亚 + 枪手/狙神 | 完美谢幕：4 次强化普攻，第 4 炮额外伤害 |
| Rumble | 约德尔人 + 护卫 | 机械重组：获得持续 4 秒护盾 + 锥形范围魔法伤害 |
| Sona | 德玛西亚 + 神谕者 | 能量和弦：对 2 名敌人魔法伤害 + 治疗最低血量友军 |

**2 费英雄（cost2/）**：

| 英雄 | 羁绊 | 技能 |
|------|------|------|
| Ashe | 弗雷尔卓德 + 迅捷 | 臻冰之箭：范围伤害 + 低于 30%HP 真实伤害 + 3 秒冰冷 |
| Cho'Gath | 虚空 + 主宰 | 碎裂：范围击飞 1.5 秒 + 魔法伤害 + 自回血 |
| Xin Zhao | 德玛西亚/艾欧尼亚 + 神盾使 | 三重爪击：3 连击 + 自回血 + 最后一击 1.5 秒晕眩 |
| Yasuo | 艾欧尼亚 + 裁决 | 踏前斩：冲刺 + 范围伤害，单目标双倍 |

**3 费英雄（cost3/）**：

| 英雄 | 羁绊 | 技能 |
|------|------|------|
| Ahri | 艾欧尼亚 + 法师 | 狐火：3 团狐火，每第 3 次施放变为 9 团 |
| Jinx | 祖安 + 枪手 | 枪炮交响曲：被动切换鱼骨头，发射 3 枚导弹 |
| Loris | 皮尔特沃夫 + 神盾使 | 皮城争斗：持续 4 秒护盾 + 冲锋击退 + 1.25 秒晕眩 |
| Sejuani | 弗雷尔卓德 + 护卫 | 凛冬之怒：持续 4 秒护盾 + 范围 4 秒冰冷 + 对已冰冷目标 1 秒晕眩 |


## 4. 关键算法与系统实现

### 4.1 阶段循环与战斗状态机

游戏流程由 `Game::m_phase`（`GamePhase::Preparation` / `GamePhase::Combat`）控制：

- **Preparation → Combat**：玩家点击 "Start Combat" 触发 `startCombat()`（`game.cpp:1834`）：锁定金币快照、创建战斗副本单位、部署敌方阵容、施加羁绊加成、启动 200ms 战斗定时器。
- **Combat → Preparation**：战斗胜负确定后 `finishCombat()`（`game.cpp:2535`）：结算基础奖励（4 + 轮次-1 金币）+ 利息 + 连败补偿、胜方额外获得与基础奖励等额金币、败方扣血（扣血量 = 胜方存活单位数，最低 1）、30% 概率掉落装备、推进轮次、恢复备战棋盘。

**单位状态机**（`UnitState` 枚举，`unit.h:8`）：

| 状态 | 含义 | 触发条件 |
|------|------|----------|
| `Idle` | 空闲 | 被眩晕、攻击冷却中、无目标可攻击 |
| `Moving` | 移动中 | 有目标但不在攻击范围内，移动冷却 = 300ms |
| `Attacking` | 攻击中 | 在攻击范围内 + 攻击冷却完成 |
| `Casting` | 施法中 | 法力值满 |
| `Dead` | 死亡 | HP ≤ 0 |

每帧 `combatTick()`（`game.cpp:2471`）逐单位检查：HP ≤ 0 → Dead；被眩晕 → Idle；攻击冷却中 → Idle；法力满 → Casting 释放技能；在攻击范围内 → Attacking 普攻；否则 → Moving，BFS 寻路移动一步。目标死亡后回到 Idle 并在下一帧重新索敌。

### 4.2 敌人生成与轮次推进

敌方每轮由 `runEnemyPrepareAgent()`（`game.cpp:3009`）自动运营：优先升级人口（金币充裕时）→ 部署备战区已有单位到上半场（第 0-3 行）→ 多轮购买高分商店单位 → 再次部署 → 调用 `arrangeEnemyFormation()` 按 Frontline/Backline 布阵。敌方随轮次推进自然增强（AI 积累的金币和单位越多，阵容越强）。

### 4.3 目标选择算法

单位通过 `findNearestEnemy()` 选择攻击目标，由 `targetThreatScore()` 辅助计算威胁评分，综合考量：欧氏距离（越近越高）、当前血量百分比（越低越高）、攻击力（越高越高）、是否为远程（近战额外加分）、是否正在攻击自身（额外加分）。选择威胁评分最高的目标；同分时按距离 → 血量 → 坐标序打破平局。

### 4.4 BFS 寻路与防重叠

`moveUnitTowardTarget()`（`game.cpp:2695`）实现 BFS 寻路：

1. 检查单位是否已在目标攻击范围内（是则无需移动）
2. 以当前位置为起点，使用双端队列进行 BFS，搜索 6 个方向（上下左右 + 两个对角）
3. 跳过已被其他单位占用的格子（`m_board.hasUnitAt()`）和已访问的格子
4. 使用 `parent` 哈希表记录前驱节点，便于回溯
5. 找到目标攻击范围内距离最近的可达格子后停止搜索
6. 沿 parent 从目标格子回溯到起点，取第一步作为移动方向



### 4.5 普攻、回蓝与技能释放

1. **普攻**（`performAttack()`）：计算伤害为：ATK - 目标护甲减免，暴击时伤害翻倍。其中，Jhin 的强化普攻特殊处理。Jinx 在 `empoweredShots` 状态下改为发射 3 枚导弹。
2. **回蓝**：每次普攻回复 10 点法力值（`game.cpp:2389`）。
3. **技能释放**：法力值 ≥ 最大法力值时，`combatTick()` 设为 Casting，调用 `castSkill()` → `caster->castSkill(this, target)`（虚函数分发），法力值清零（各英雄的 `castSkill()` 开头调用 `setMana(0)`）。
4. **死亡**：HP ≤ 0 时设为 Dead，`removeDeadUnits()` 从棋盘移除。

### 4.6 商店与经济系统

初始金币为 6（`kInitialGold = 6`）。每轮结算时金币来源：

- 每轮基础奖励：4 + (轮次-1) 金币
- 利息：每 10 金币额外 +1，上限 5 金币（即存 50+ 金币时达到最大值，`interestForGold()`，`game.cpp:195`）
- 连败补偿：0-1 连败为 0，2 连败 +2 金币，3 连败 +4 金币，≥4 连败 +8 金币（`lossStreakCompensation()`，`game.cpp:181`）
- 胜利额外奖励：与基础轮次奖励等额

**商店**：`rollShopFor()` 从商店池随机选取，当前 `kShopSlotCount = 2`（`game.cpp:1444`）。购买扣除对应金币，单位进入备战区并触发升星检查。刷新商店花费 4 金币（`kShopRefreshCost = 4`）。

### 4.7 人口系统

- 人口上限 = 玩家等级（`m_level`），初始为 1。
- 升级通过 `buyLevelProgress()` 每次消耗 4 金币获得 4 点进度；各等级所需升级进度为 {4, 4, 6, 10, 15, 30, 36, 60, 70}（`kLevelUpCosts`，`player.h:51`），进度满后自动升级。
- 上阵时检查 `deployedPlayerUnitCount() < m_player.unitCap()`。
- 备战区不受人口限制（固定 8 格）。

### 4.8 羁绊系统

共实现 6 种羁绊，通过检查战斗副本中单位的标签计数激活：

**属性光环类（4 种）**：

| 羁绊 | 类型 | 激活人数 | 效果 |
|------|------|----------|------|
| 德玛西亚 | Origin | 2 / ≥3 | 德玛西亚单位 HP +100 / +180 |
| 艾欧尼亚 | Origin | 2 / ≥3 | 艾欧尼亚单位攻速 ×1.10 / ×1.20 |
| 弗雷尔卓德 | Origin | ≥2 | 弗雷尔卓德单位护甲 +20，魔抗 +20 |
| 枪手 | Role | ≥2 | 枪手单位 ATK +15 |

**机制改变类（2 种）**：

| 羁绊 | 类型 | 激活人数 | 效果 |
|------|------|----------|------|
| 护卫 | Role | 2 / ≥3 | 护卫单位获得 120 / 220 护盾（护盾持续整场战斗） |
| 神盾使 | Role | ≥2 | 神盾使单位获得 180 护盾（护盾持续整场战斗） |

实现位于 `computeActiveSynergies()`（`game.cpp:2069`），通过 lambda 闭包统计数量并施加 buff。羁绊仅在战斗副本上施加（`applyCombatSynergies()`），不影响备战阵容。

### 4.9 升星系统

`tryMergeUnit()`（`game.cpp:1611`）实现 3 合 1 升星：

1. 购买单位后自动调用
2. `findMergeCandidates()` 查找所有同名、同星级的单位（棋盘 + 备战区）
3. 候选数 ≥ 3 时触发合并：优先保留棋盘上的单位
4. 删除 2 个，保留 1 个 → 星级 +1、费用 ×3、最大 HP ×2 并回满、ATK ×2、护甲 ×2

### 4.10 装备系统

**基础装备（11 种）**：

| 装备 | 文件定义 | 效果 |
|------|----------|------|
| 暴风大剑 | `EquipmentType::BFSword` | ATK +10 |
| 反曲之弓 | `EquipmentType::RecurveBow` | 攻速 +10% |
| 无用大棒 | `EquipmentType::NeedlesslyLargeRod` | 法强 +10 |
| 女神之泪 | `EquipmentType::TearOfTheGoddess` | 法力 +15 |
| 锁子甲 | `EquipmentType::ChainVest` | 护甲 +20 |
| 负极斗篷 | `EquipmentType::NegatronCloak` | 魔抗 +20 |
| 巨人腰带 | `EquipmentType::GiantsBelt` | HP +150 |
| 拳套 | `EquipmentType::SparringGloves` | 暴击率 +20% |
| 铁剑 | `EquipmentType::IronSword` | ATK +15 |
| 急速手套 | `EquipmentType::HasteGloves` | 攻速 +20% |
| 蓝水晶 | `EquipmentType::BlueCrystal` | 最大法力 -30 |

其中铁剑（IronSword, ATK+15）、锁子甲（ChainVest, 护甲+20）、急速手套（HasteGloves, 攻速+20%）、蓝水晶（BlueCrystal, 最大法力-30）覆盖了 PA 文档要求的 4 种基础装备类型。

**装备合成（4 种成装）**：

| 成装 | 配方 | 被动效果 |
|------|------|----------|
| 鬼索的狂暴之刃 | 暴风大剑 + 反曲之弓 | 每次普攻后攻速永久提升 6%（可叠加） |
| 朔极之矛 | 暴风大剑 + 女神之泪 | 每次普攻额外回复 5 点法力 |
| 荆棘背心 | 锁子甲 + 锁子甲 | 被暴击时反弹 25 点伤害；暴击伤害对穿戴者降低 10% |
| 狂徒铠甲 | 巨人腰带 + 巨人腰带 | 脱离战斗（3 秒未受击）后每秒回复 5% 最大生命值 |

合成在 `equipInventoryItem()` 中自动检测（`combinedEquipment()` 判断配方）。

**掉落**：战斗胜利后，`tryDropBasicEquipment()` 以 30% 概率从 11 种基础装备中随机掉落 1 件到装备栏（装备栏满 8 件时不触发）。

**穿戴限制**：每单位装备容量 = 星级（1 星 1 件，2 星 2 件，3 星 3 件，`equipmentCapacity()`）。

### 4.11 存档与读档

**存档**（`saveGame()`，`game.cpp:350`）：将完整游戏状态序列化为 JSON 对象，写入文件。保存字段包括 `version`、`phase`、`gameOver`、`player`/`enemy` 完整数据、`shop`/`enemyShop` 槽位、`equipmentInventory`、所有单位完整快照（属性、位置、owner、装备、状态等）。

**读档**（`loadGame()`，`game.cpp:548`）：解析 JSON 文件，重建 Player、Board、Bench、Unit、装备库存等全部状态，重新创建 GUI 图形项，调用 `syncFromState()` 刷新界面。存档文件格式为 `.json`。


## 5. 辅助函数说明

以下为项目中的重要辅助函数，按功能分类说明：

**羁绊与文本转换**：
- `originToText()` / `roleToText()`：将 `Origin` / `Role` 枚举值转为中英文显示文本。
- `heroLocalizedName()`：将英雄英文名（如 "Jarvan IV"）转为中文名。
- `equipmentShortName()` / `equipmentName()` / `equipmentStatText()`：返回装备的简称/全称/属性描述。

**UI 文本生成**（`src/gui/`）：
- `shopStatsText()`：生成商店卡牌中单位参数的 HTML 富文本。
- `unitStatsText()`：生成右键详情卡中完整属性的 HTML 富文本。
- `skillText()` / `unitSkillText()`：拆分技能名和技能描述正文。

**坐标转换**（`src/core/game.cpp`）：
- `gridToWorld()`：将棋盘逻辑坐标 (row, col) 转换为场景像素坐标。
- `worldToGrid()`：将场景像素坐标转换为最近棋盘格子坐标。
- `benchSlotRect()` / `benchSlotCenter()`：计算备战区槽位的矩形区域和中心像素坐标。
- `findBenchSlot()`：根据鼠标场景坐标判断落在哪个备战区槽位。

**单位管理**（`src/core/game.cpp`）：
- `removeUnitCompletely()`：从棋盘、备战区、图形 item 列表、单位列表中彻底删除一个单位。
- `findMergeCandidates()`：查找与指定单位同名同星级的候选，用于升星合并。
- `cloneUnitForCombat()`：深拷贝一个单位，用于创建战斗副本。

**装备相关**（`src/core/game.cpp`）：
- `tryDropBasicEquipment()`：胜利结算时以 30% 概率随机掉落 1 件基础装备。
- `applyEquipmentStats()`：将装备的属性加成应用到单位的对应字段。
- `combinedEquipment()`：判断两件散件是否满足合成配方，返回合成的成装类型。

**战斗辅助**（`src/core/game.cpp`）：
- `setupCombatCopies()` / `restorePreparationBoard()`：进入/退出战斗时创建/销毁战斗副本并恢复备战阵容。
- `starredValue()`：根据星级（1-3）从三元素数组中取出对应值，用于技能伤害/治疗/护盾的星级缩放。
- `targetThreatScore()`：计算敌方单位对攻击者的威胁评分，综合距离、血量、攻击力、远程等因素。
- `effectiveAttackSpeed()`：计算单位的实际攻击速度，考虑攻速加成和冰冷减速。
- `updateCombatEffects()`：每 tick 递减单位的眩晕、冰冷、攻速加成、护盾持续时间等战斗效果。

**敌方 AI**（`src/core/game.cpp`）：
- `runEnemyPrepareAgent()`：敌方准备阶段主逻辑：升级 → 部署 → 购买 → 合成 → 布阵。
- `arrangeEnemyFormation()`：按 Frontline/Backline 类型将敌方单位排列到上半场合适位置。
- `enemyShopSlotScore()`：对商店槽位评分，考虑费用、属性、羁绊匹配度。

**存档相关**（`src/core/game.cpp`）：
- `saveGame()` / `loadGame()`：将完整游戏状态序列化为 JSON 并写入文件 / 从文件读取并恢复。

## 6. 面向对象与多态设计

### 6.1 统一单位模型

`src/entity/unit/unit.h` 定义 Unit 基类，包含所有战斗单位共有的属性（HP、ATK、法力、装备等）和行为接口（`castSkill()`）。我方单位与敌方单位使用同一 Unit 类型，仅通过 `m_owner` 字段（`Owner::PlayerCtrl` / `Owner::EnemyCtrl`）区分归属。`m_origins` 和 `m_roles` 仅用于羁绊计算，不用于敌我区分。严格遵守 PA 文档中 "owner 标记敌我归属，traits 标记羁绊标签" 的形式化要求。

### 6.2 继承体系

12 个英雄类均继承自 Unit，各自维护 `.h` 声明和 `.cpp` 实现文件。每个英雄类在构造函数中设置自身的基础属性，并重写虚函数 `void castSkill(Game* game, Unit* target)`。

### 6.3 虚函数与动态绑定

- 基类声明：`src/entity/unit/unit.h:151` — `virtual void castSkill(Game* game, Unit* target) {}`
- 派生类重写：如 `src/entity/heros/cost1/Jhin.cpp` 中 Jhin 实现完美谢幕；`src/entity/heros/cost3/Ahri.cpp` 中 Ahri 实现狐火。
- 调用点：`src/core/game.cpp:2302` — `Game::castSkill(Unit* caster, Unit* target)` 中调用 `caster->castSkill(this, target)`。
- 运行时效果：`combatTick()` 在法力满时调用 `castSkill(attacker, target)`，C++ 虚函数表根据 `attacker` 的实际类型自动分发到正确的重写版本，无需 if-else 或 switch 判断英雄类型。

### 6.4 工厂模式

`Game::createUnitFromShopSlot()`（`game.cpp:1508`）使用 switch 根据 `HeroType` 枚举创建对应的英雄派生类对象，返回 `Unit*` 基类指针，保证商店/存档等模块只依赖 Unit 接口而不关心具体英雄类型。

## 7. 项目简介

Synera 是一款单机版轻量级自走棋游戏（PvE），核心玩法分为三个阶段循环：

- **准备阶段（Prep）**：玩家使用金币在商店购买英雄、刷新商店、将英雄从备战区拖拽至棋盘上阵、调整站位、穿戴装备、升级人口上限。
- **战斗阶段（Combat）**：敌方单位根据轮次自动生成并部署于上半场。双方单位自动索敌、移动、普攻回蓝、释放技能。玩家不可操作。
- **结算阶段（Resolve）**：战斗结束后根据胜负结算金币（含利息和连败补偿）、血量、轮次推进，30% 概率掉落基础装备。玩家或敌方血量归零则游戏结束。

游戏支持 JSON 格式的存档与读档，可随时保存/恢复完整游戏状态。敌方 AI 在每轮准备阶段自动进行购买、刷新、升级、出售、升星和布阵决策。

## 8. 编译与运行方式

1. 使用 CLion 打开项目根目录 `Synera_Starter`。
2. 确保本机已安装 Qt6（包含 Core、Widgets、Gui 模块）。
3. CLion 自动识别 `CMakeLists.txt` 并加载 CMake 项目。
4. 选择运行目标 `Synera_Starter`。
5. 点击 Build 编译，再点击 Run 运行。

命令行直接编译需要手动配置 Qt6 路径（`CMAKE_PREFIX_PATH`），推荐使用 CLion 避免路径配置问题。

## 9. 游戏玩法说明

1. **启动游戏**：游戏启动后自动进入准备阶段，玩家获得初始金币 6 和 1 个随机 1 费英雄。
2. **商店购买**：左侧商店区域显示当前可购买的英雄。点击英雄卡片扣除对应金币，英雄进入备战区。点击 "Refresh 4 gold" 按钮花费 4 金币刷新商店。
3. **上阵**：将英雄从备战区拖拽到下方玩家半场（第 5-8 行）。拖拽到已有单位的地块上会自动交换位置。拖拽回备战区则下阵。拖拽到左下角出售区则出售单位（返还等额金币）。
4. **开始战斗**：点击 "Start Combat" 按钮，系统自动在敌方半场生成敌人，进入战斗阶段。
5. **自动战斗**：双方单位自动索敌、移动、普攻（回蓝）、释放技能。界面显示弹道飞行特效。
6. **结算**：敌方全灭则玩家胜利（获得金币+利息、可能掉落装备），我方全灭则玩家失败（扣血 = 胜方存活单位数，最低 1）。进入下一轮准备阶段。
7. **游戏结束**：任一方血量归零时游戏结束，弹出 Victory/Defeat 对话框。
8. **其他功能**：升级人口（每次 4 金币获得进度）、右键查看单位详情、装备拖拽穿戴与合成、存档读档。

**商店槽位设计说明***：当前商店每次刷出 2 个英雄而非 PA 要求的 5 个。这是有意为之的设计选择——游戏共 12 名英雄，5 格商店将严重降低运营深度和筛选趣味性。如需满足 PA Checklist，将 `src/core/game.cpp:1444` 的 `kShopSlotCount` 从 2 改为 5 即可。

## 10. PA 要求完成度对照表

| PA 要求 | 完成状态 | 对应文件/类 | 验收方式 |
|-------|-----|-------------|----------|
| **阶段一** |     | | |
| M×N 棋盘、玩家/敌方半场、地块占用 | 已完成 | `src/core/board.h`，ROWS=COLS=8 | 拖拽单位到不同半场测试占用 |
| 备战区与棋盘同步 | 已完成 | `src/core/bench.h`，SLOTS=8 | 拖拽单位在棋盘和备战区之间切换 |
| Unit 基类（HP/ATK/Range/Mana） | 已完成 | `src/entity/unit/unit.h:91-186` | 查看单位属性面板 |
| owner 区分敌我，traits 用于羁绊 | 已完成 | `unit.h:25-86` 枚举 Owner/Origin/Role | 敌方单位为红色，羁绊栏显示标签 |
| Player 实体 | 已完成 | `src/core/player.h` | 状态栏显示 HP/Gold/Level |
| 敌方轮次生成 | 已完成 | `game.cpp:3009` runEnemyPrepareAgent() | 战斗开始时敌方半场自动出现单位 |
| 拖拽摆放与非法放置处理 | 已完成 | `game.cpp:751` handleDropCommand()，含交换/回弹/出售区 | 拖拽到非法位置自动弹回或交换 |
| GUI 展示棋盘/备战区/单位信息 | 已完成 | `src/gui/gamewindow.cpp` | 界面展示血条、蓝条、属性面板 |
| **阶段二** |     | | |
| Prep/Combat/Resolve 三阶段循环 | 已完成 | `game.cpp` startCombat()/combatTick()/finishCombat() | 点击 Start Combat → 战斗 → 自动结算 |
| 敌方单位按轮次生成并增强 | 已完成 | `game.cpp` runEnemyPrepareAgent() + advanceRound() | 观察多轮后敌方阵容变化 |
| Unit 状态机 | 已完成 | `unit.h:8` UnitState + `game.cpp:2471` combatTick() | 战斗中观察单位状态切换 |
| 索敌规则（距离+平局优先级） | 已完成 | `game.cpp` findNearestEnemy() + targetThreatScore() | 观察单位选择攻击目标的行为 |
| BFS 寻路、阻挡、防重叠 | 已完成 | `game.cpp:2695` moveUnitTowardTarget() | 单位绕过阻挡到达目标 |
| 普攻、回蓝 | 已完成 | `game.cpp` performAttack()，每普攻+10 法力 | 观察蓝条增长 |
| 多态技能（3-5 英雄） | 已完成 | 12 个英雄类，`src/entity/heros/` | 战斗中观察不同英雄释放不同技能 |
| 胜负结算 | 已完成 | `game.cpp:2535` finishCombat() | 一方全灭后自动结算 |
| **阶段三** |     | | |
| 金币系统 | 已完成 | `player.h` + 利息/连败补偿 | 状态栏显示金币变化 |
| 商店招募位 | 已完成 | `game.cpp:1444` kShopSlotCount=2 | 改为 5 即可满足要求 |
| 购买、刷新、备战区落位 | 已完成 | `game.cpp:1477` buyShopUnit() | 点击商店卡片→单位进入备战区 |
| 人口上限与升级 | 已完成 | `player.cpp` buyLevelProgress()，unitCap()=level | 点击 Upgrade Level 升级 |
| 4-6 种羁绊 | 已完成 | 6 种：3 Origin + 3 Role | 羁绊状态栏显示激活信息 |
| ≥2 种属性光环类羁绊 | 已完成 | 德玛西亚(HP)、艾欧尼亚(攻速)、弗雷尔卓德(双抗)、枪手(ATK) | 查看激活时的属性变化 |
| ≥1 种机制改变类羁绊 | 已完成 | 护卫(护盾)、神盾使(护盾) | 查看激活时的护盾效果 |
| 升星 3 合 1 | 已完成 | `game.cpp:1611` tryMergeUnit() | 购买第 3 个同名单位自动合并 |
| 2 星属性提升 | 已完成 | HP×2, ATK×2, 护甲×2 | 查看 2 星单位面板 |
| 装备掉落 | 已完成 | `game.cpp` tryDropBasicEquipment()，30% 概率 | 胜利后有概率掉落 |
| 装备穿戴限制 | 已完成 | 容量=星级，equipmentCapacity() | 拖拽装备到单位穿戴 |
| ≥4 种基础装备 | 已完成 | 11 种基础 + 4 种成装 = 15 种 | 查看装备栏和单位面板 |
| 存档读档 | 已完成 | `game.cpp:350` saveGame() / loadGame()，JSON 格式 | 保存后重开游戏读档验证 |
| GUI 展示经济/商店/羁绊/星级/轮次/阶段 | 已完成 | `gamewindow.cpp` 状态栏+商店卡片+详情卡 | 界面各项信息完整可读 |
| **阶段四** |     | | |
| 利息机制 | 已完成 | `game.cpp:195` interestForGold()，每 10 金+1，上限 5 | 金币 ≥10 时结算显示额外 +1 |
| 连败补偿 | 已完成 | `game.cpp:181` lossStreakCompensation() | 连败后结算金币增加 |
| 装备合成树 | 已完成 | `game.cpp` combinedEquipment()，4 种成装 | 穿戴两件配方散件自动合成 |
| 弹道特效 | 已完成 | `game.cpp` showAttackProjectile() / updateAttackProjectiles() | 战斗中显示攻击飞行弹道 |
| 智能索敌 | 已完成 | `game.cpp` targetThreatScore() 多维度评分 | 观察单位优先攻击高威胁目标 |


## 11. AI 使用说明

### 11.1 AI 辅助范围与具体贡献

本项目使用 Codex CLI 进行辅助开发。大部分代码由本人完成，交由 Codex 进行验收，以确保代码的正确性。一部分功能的实现由 Codex 完成。

以下按模块说明 AI 的具体贡献：

**项目架构与模块拆分**：
- AI 将 PA 需求分解为 14 个可执行子任务：棋盘渲染 → Bench 槽位 → 拖拽交互 → 单位基类 → Player 经济 → 商店购买/刷新 → 人口升级 → 战斗状态机 → 索敌 → BFS 寻路 → 技能多态（12 英雄） → 羁绊计算 → 升星合并 → 装备掉落/穿戴/合成 → JSON 存档
- 本人按此顺序逐模块实现，每完成一个模块后运行验证。

**棋盘与 GUI 层**：
- `src/gui/gamewindow.cpp`（1226 行）：AI 辅助生成了商店卡片 HTML 富文本布局、装备图谱对话框的网格排列、状态栏样式

**战斗系统**：
- `combatTick()`（`game.cpp:2498`）：AI 提供了按优先级检查状态机的循环框架（Dead → Stunned → Cooldown → Casting → Attacking → Moving），本人手动填充了每个分支的冷却时间计算和 `setState()` 调用
- `performAttack()`（`game.cpp:2357`）：AI 生成基础普攻逻辑
- `updateCombatEffects()`（`game.cpp:2426`）：AI 生成 stun/chill/attackSpeed 的逐 tick 递减框架。**本人发现并修复了关键 bug**：护盾（`shield`）没有任何过期机制，导致 Loris/Sejuani 等英雄获得永久护盾。本人新增了 `shieldSeconds` 字段到 `CombatUnitState`（`game.h:96`），在 `updateCombatEffects()` 中添加 `if (shield > 0 && shieldSeconds > 0.0)` 衰减逻辑（`game.cpp:2439-2443`），并在 4 个护盾英雄的 `castSkill()` 中设置 `shieldSeconds = 4.0`
- `cloneUnitForCombat()`（`game.cpp:1906`）：**本人发现并修复了致命 bug**——AI 生成的代码使用 `new Unit(name)` 创建裸基类对象，导致战斗副本的 `castSkill()` 虚函数分发落到基类空实现上，所有英雄技能在战斗中完全不生效（蓝条不扣、伤害不触发）。本人将其改为 `switch (heroType)` 创建对应派生类（`new Loris()` 等），使虚函数表正确指向各英雄的 `castSkill()` 重写

**敌方 AI**：
- `runEnemyPrepareAgent()`（`game.cpp:3036`）：AI 提供"升级 → 部署 → 购买 → 布阵"的主循环结构。本人添加了 5 轮购买 `pass` 循环、金币阈值判断（`gold >= 24` 才升级）、以及 `enemyShopSlotScore()` 评分函数的具体权重
- `enemyShopSlotScore()`（`game.cpp:2835`）：评分公式 `cost*100 + star*250 + maxHp/10 + atk` + 同名单位合并奖励（2 个匹配 +180，3 个匹配 +1000），本人设计了权重系数以平衡 AI 的购买倾向

**装备系统**：
- `equipInventoryItem()`（`game.cpp:1744`）：AI 生成穿戴逻辑框架，本人添加了散件合成检测（遍历已穿装备 → 调用 `combinedEquipment()` → 替换标识）和装备容量检查（`equipmentCapacity() = star`）

**JSON 存档**：
- `saveGame()`（`game.cpp:350`）：AI 生成 JSON 序列化模板，将 `Player`、商店、装备库存、所有单位属性（含 `CombatUnitState` 的 `shield`/`stunSeconds`/`chillSeconds`/`attackSpeedBonus`/`empoweredShots`/`basicAttackCount`/`skillCastCount`/`shieldSeconds`）写入文件



### 11.2 核心模块解释一：BFS 寻路

**代码位置**：`src/core/game.cpp:2721-2796`，`Game::moveUnitTowardTarget()`

BFS（广度优先搜索）用于在棋盘网格上找到从单位当前位置到目标攻击范围内最近可达格子的最短路径，替代早期版本中的贪心直线移动。

**数据结构**：
- `std::deque<QPoint> queue`：BFS 搜索队列，`push_back` 入队，`pop_front` 出队
- `std::unordered_map<int, QPoint> parent`：前驱节点哈希表，key 为 `x*100+y` 将二维坐标压为一维索引，value 为到达该格子的上一步坐标
- `static const std::array<QPoint, 6> kDirections`：6 个搜索方向（上下左右 + 右下/左上两个对角）

**搜索流程**：
1. 起点入队，`parent[start] = start` 标记已访问
2. 循环出队当前节点 `cur`，计算 `cur` 到目标 `targetPos` 的距离平方 `distSq`
3. 若 `distSq ≤ attackRange²` 且比当前最佳 `bestDistSq` 更近：记录 `bestGoal = cur`，标记 `found = true`，继续同层搜索（不立即停止，因为同层可能有更优目标）
4. 若 `found && distSq ≥ bestDistSq`：当前层已无更优解，跳过扩展（剪枝优化）
5. 否则对 6 个方向的相邻格子：跳过越界（`!isValidPosition`）、跳过被占（`hasUnitAt && != start`）、跳过已访问（`parent.count`）；合法格子设置前驱并入队
6. 队列空后，若 `found && bestGoal != start`：沿 `parent` 回溯 `while (parent[step] != start) step = parent[step]`，`step` 即为第一步移动目标
7. `m_board.removeUnit(unit)` → `m_board.addUnit(unit, step)` → `syncFromState()` 完成一步移动

**算法特性**：BFS 从起点逐层向外扩展，天然保证找到的是最短路径（按步数）。当所有方向均被阻挡时 `bestGoal == start`，函数直接 return 不移动。

### 11.3 核心模块解释二：技能多态与动态绑定

**代码位置**：
- 基类虚函数声明：`src/entity/unit/unit.h:151` — `virtual void castSkill(Game* game, Unit* target) {}`
- 多态调用点：`src/core/game.cpp:2330` — `caster->castSkill(this, target);`
- 派生类示例：`src/entity/heros/cost3/Ahri.cpp:32-41`、`src/entity/heros/cost1/Jhin.cpp:32-36`

**设计原理**：本项目使用 C++ 虚函数机制实现技能多态。Unit 基类声明 `virtual void castSkill(Game*, Unit*)`，提供默认空实现 `{}`。12 个英雄派生类各自重写此函数。

**运行时流程**：
1. `combatTick()` 检测 `attacker->mana() >= attacker->maxMana()`，调用 `castSkill(attacker, target)`
2. `Game::castSkill()`（`game.cpp:2323`）调用 `caster->castSkill(this, target)` — 注意参数是 `Unit* caster`（基类指针）
3. C++ 运行时通过虚函数表（vtable）查找 `caster` 实际指向对象的类型。若 `caster` 实际是 `Ahri*`，则跳转到 `Ahri::castSkill()`；若实际是 `Loris*`，则跳转到 `Loris::castSkill()`
4. 各派生类的实现完全不同：Ahri 对 3 个附近敌人造成魔法伤害（`skillAreaTargets(target, enemyOwner, 3)` → `dealDamage({82,125,225})`），Loris 获得 4 秒护盾 + 对目标 1.25 秒晕眩 + 魔法伤害（`shield += {700,800,1000}` + `shieldSeconds = 4.0` + `stunSeconds = 1.25` + `dealDamage({150,225,360})`）

### 11.4 核心模块解释三：战斗状态机与 combatTick

**代码位置**：`src/entity/unit/unit.h:8`（`UnitState` 枚举）+ `src/core/game.cpp:2498`（`combatTick()`）

战斗循环通过 `QTimer` 每 200ms 触发一次 `combatTick()`。该函数对双方所有存活单位执行统一的状态机推进。

**状态定义与转换**（`unit.h:8-14`）：
- `Idle`：空闲 — 被眩晕、冷却中、无目标时的默认状态
- `Moving`：移动 — 有目标但超出攻击范围，调用 `moveUnitTowardTarget()` 执行 BFS 寻路一步
- `Attacking`：攻击 — 目标进入攻击范围，调用 `performAttack()` 造成伤害并回蓝
- `Casting`：施法 — 法力满，调用 `castSkill()` 释放技能，冷却 = `1.0/effectiveAttackSpeed()`
- `Dead`：死亡 — HP ≤ 0，下一 tick 由 `removeDeadUnits()` 清理

**combatTick() 执行流程**（`game.cpp:2498-2555`）：
1. `updateCombatEffects()`：递减所有单位的 `stunSeconds`/`chillSeconds`/`attackSpeedBonusSeconds`/`shieldSeconds`（过期清零）
2. 遍历 `deployedUnits(PlayerCtrl) + deployedUnits(EnemyCtrl)` 的并集
3. 对每个 `attacker` 按优先级检查：
   - HP ≤ 0 → `setState(Dead)`，`continue`
   - `stunSeconds > 0` → `setState(Idle)`，`continue`（眩晕期间不行动）
   - `cooldown > 0` → `setState(Idle)`，递减冷却，`continue`
   - `mana >= maxMana && maxMana > 0` → `setState(Casting)` → `castSkill()` → 冷却 = `1.0 / effectiveAttackSpeed()`
   - `isInAttackRange(target)` → `setState(Attacking)` → `performAttack()` → 冷却 = `1.0 / effectiveAttackSpeed()`
   - 否则 → `setState(Moving)` → `moveUnitTowardTarget()` → 冷却 = `300ms`
4. `updateWarmogsHealing()`：狂徒铠甲回血（脱离战斗 3 秒后每秒 5% 最大 HP）
5. `removeDeadUnits()`：移除 HP ≤ 0 的单位
6. 检测战斗结束：任一方场上无存活单位 → `finishCombat()`

## 12. 已知问题与后续改进

- **商店槽位**：当前为 2 格而非 PA 要求的 5 格。设计理由见第 9 节。如需通过 Checklist 验收，将 `src/core/game.cpp:1444` 的 `kShopSlotCount` 改为 5 即可。
- **GUI 美术资源**：当前使用简单纯色方块 + 文字标注，未使用精细像素素材。
- **平衡性**：英雄数值和羁绊效果未经过系统性平衡测试。（不过参数都是照抄金铲铲的）
- **BFS 寻路**：对极端拥堵场景可能无路可走，当前原地等待。
- **敌方 AI**：使用基于分数的贪心策略，后续可以改为使用 MinMax 等搜索算法。
- **音效**：未添加背景音乐或攻击/技能音效。

