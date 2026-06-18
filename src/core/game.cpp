#include "game.h"
#include "entity/unit/unit.h"
#include "gui/griditem.h"
#include "gui/equipmentitem.h"
#include "gui/unititem.h"
#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QFont>
#include <QPainterPath>
#include <QPen>
#include <QSaveFile>
#include <QTimer>
#include <deque>
#include <QtMath>
#include <random>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <utility>


#include "entity/heros/cost1/JarvanIV.h"
#include "entity/heros/cost1/Jhin.h"
#include "entity/heros/cost1/Rumble.h"
#include "entity/heros/cost1/Sona.h"
#include "entity/heros/cost2/Ashe.h"
#include "entity/heros/cost2/ChoGath.h"
#include "entity/heros/cost2/XinZhao.h"
#include "entity/heros/cost2/Yasuo.h"
#include "entity/heros/cost3/Ahri.h"
#include "entity/heros/cost3/Jinx.h"
#include "entity/heros/cost3/Loris.h"
#include "entity/heros/cost3/Sejuani.h"

namespace {
constexpr qreal kZGrid = 0.0;
constexpr qreal kZUnit = 1.0;
constexpr qreal kZDraggingUnit = 2.0;
constexpr qreal kBenchSlotSize = 80.0;
constexpr qreal kBenchSlotGap = 6.0;
constexpr qreal kBenchRowGap = 14.0;
constexpr qreal kEquipmentSlotWidth = 76.0;
constexpr qreal kEquipmentSlotHeight = 50.0;
constexpr qreal kEquipmentSlotGap = 6.0;
constexpr qreal kEquipmentColumnGap = 12.0;
constexpr int kCombatTickMs = 200;
constexpr double kCombatTickSeconds = kCombatTickMs / 1000.0;
constexpr int kProjectileTickMs = 30;
constexpr int kProjectileLifetimeTicks = 6;
constexpr double kMoveCooldownSeconds = kCombatTickSeconds * 1.5;
constexpr int kEquipmentSlotCount = 8;
constexpr int kPreparationDurationSeconds = 10 * 60;

qint64 euclideanDistanceSquared(const QPoint& from, const QPoint& to)
{
    const qint64 dx = static_cast<qint64>(from.x()) - to.x();
    const qint64 dy = static_cast<qint64>(from.y()) - to.y();
    return dx * dx + dy * dy;
}

std::array<QPoint, 6> hexNeighborOffsets(int row)
{
    if (row % 2 == 0) {
        return {
            QPoint(-1, 0), QPoint(1, 0), QPoint(0, -1),
            QPoint(1, -1), QPoint(0, 1), QPoint(1, 1)
        };
    }
    return {
        QPoint(-1, 0), QPoint(1, 0), QPoint(-1, -1),
        QPoint(0, -1), QPoint(-1, 1), QPoint(0, 1)
    };
}

int gridIndex(const QPoint& pos)
{
    return pos.y() * Board::COLS + pos.x();
}

int hexGridDistance(const QPoint& from, const QPoint& to)
{
    if (!Board::isValidPosition(from) || !Board::isValidPosition(to)) {
        return std::numeric_limits<int>::max();
    }
    if (from == to) {
        return 0;
    }

    std::array<int, Board::ROWS * Board::COLS> distances{};
    distances.fill(-1);

    std::deque<QPoint> queue;
    queue.push_back(from);
    distances[static_cast<std::size_t>(gridIndex(from))] = 0;

    while (!queue.empty()) {
        const QPoint current = queue.front();
        queue.pop_front();

        const int currentDistance = distances[static_cast<std::size_t>(gridIndex(current))];
        for (const QPoint& offset : hexNeighborOffsets(current.y())) {
            const QPoint next = current + offset;
            if (!Board::isValidPosition(next)) {
                continue;
            }

            int& nextDistance = distances[static_cast<std::size_t>(gridIndex(next))];
            if (nextDistance >= 0) {
                continue;
            }
            nextDistance = currentDistance + 1;
            if (next == to) {
                return nextDistance;
            }
            queue.push_back(next);
        }
    }

    return std::numeric_limits<int>::max();
}

class ClickableRectItem final : public QGraphicsRectItem
{
public:
    ClickableRectItem(const QRectF& rect, std::function<void()> onClicked)
        : QGraphicsRectItem(rect)
        , m_onClicked(std::move(onClicked))
    {
        setAcceptedMouseButtons(Qt::LeftButton);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && m_onClicked) {
            m_onClicked();
            event->accept();
            return;
        }
        QGraphicsRectItem::mousePressEvent(event);
    }

private:
    std::function<void()> m_onClicked;
};

QString equipmentShortName(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword: return "暴风大剑";
    case EquipmentType::RecurveBow: return "反曲之弓";
    case EquipmentType::NeedlesslyLargeRod: return "无用大棒";
    case EquipmentType::TearOfTheGoddess: return "女神之泪";
    case EquipmentType::ChainVest: return "锁子甲";
    case EquipmentType::NegatronCloak: return "负极斗篷";
    case EquipmentType::GiantsBelt: return "巨人腰带";
    case EquipmentType::SparringGloves: return "拳套";
    case EquipmentType::GuinsoosRageblade: return "鬼索";
    case EquipmentType::SpearOfShojin: return "朔极之矛";
    case EquipmentType::Thornmail: return "荆棘背心";
    case EquipmentType::WarmogsArmor: return "狂徒铠甲";
    case EquipmentType::IronSword: return "铁剑";
    case EquipmentType::HasteGloves: return "急速手套";
    case EquipmentType::BlueCrystal: return "蓝水晶";
    }
    return {};
}

QString equipmentStatText(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword: return "攻击 +10";
    case EquipmentType::RecurveBow: return "攻速 +10%";
    case EquipmentType::NeedlesslyLargeRod: return "法强 +10";
    case EquipmentType::TearOfTheGoddess: return "法力 +15";
    case EquipmentType::ChainVest: return "生命 +150";
    case EquipmentType::NegatronCloak: return "魔抗 +20";
    case EquipmentType::GiantsBelt: return "生命 +150";
    case EquipmentType::SparringGloves: return "暴击 +20%";
    case EquipmentType::IronSword: return "攻击 +15";
    case EquipmentType::HasteGloves: return "攻速 +20%";
    case EquipmentType::BlueCrystal: return "最大法力 -30";
    default: return "成装";
    }
}

QColor equipmentColor(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword: return {214, 102, 86};
    case EquipmentType::RecurveBow: return {104, 200, 114};
    case EquipmentType::NeedlesslyLargeRod: return {182, 111, 232};
    case EquipmentType::TearOfTheGoddess: return {82, 169, 236};
    case EquipmentType::ChainVest: return {174, 179, 190};
    case EquipmentType::NegatronCloak: return {110, 205, 213};
    case EquipmentType::GiantsBelt: return {229, 121, 108};
    case EquipmentType::SparringGloves: return {226, 194, 87};
    case EquipmentType::IronSword: return {198, 114, 108};
    case EquipmentType::HasteGloves: return {101, 211, 132};
    case EquipmentType::BlueCrystal: return {80, 151, 236};
    default: return {235, 190, 84};
    }
}

bool combinedEquipment(EquipmentType first, EquipmentType second, EquipmentType& result)
{
    if ((first == EquipmentType::RecurveBow && second == EquipmentType::NeedlesslyLargeRod)
        || (first == EquipmentType::NeedlesslyLargeRod && second == EquipmentType::RecurveBow)) {
        result = EquipmentType::GuinsoosRageblade;
        return true;
    }
    if ((first == EquipmentType::BFSword && second == EquipmentType::TearOfTheGoddess)
        || (first == EquipmentType::TearOfTheGoddess && second == EquipmentType::BFSword)) {
        result = EquipmentType::SpearOfShojin;
        return true;
    }
    if (first == EquipmentType::ChainVest && second == EquipmentType::ChainVest) {
        result = EquipmentType::Thornmail;
        return true;
    }
    if (first == EquipmentType::GiantsBelt && second == EquipmentType::GiantsBelt) {
        result = EquipmentType::WarmogsArmor;
        return true;
    }
    return false;
}

const std::vector<ShopSlot>& shopPool()
{
    static const std::vector<ShopSlot> kShopPool = {
         {HeroType::JarvanIV, "Jarvan IV", 1, 1, 700, 700, 55, 1, 100, 30, 40, 40, 0.60, 0.25, UnitPositionType::Frontline, {Origin::Demacia}, {Role::Guardian}, "德邦军旗：获得持续4秒的护盾，并给全场友军增加持续4秒的攻速。\nShield: 350 / 425 / 500\nAttack Speed Bonus: 20% / 25% / 50%"}
        ,{HeroType::Jhin, "Jhin", 1, 1, 444, 444, 44, 4, 70, 0, 30, 30, 0.70, 0.25, UnitPositionType::Backline, {Origin::Ionia}, {Role::Gunner, Role::Sniper}, "完美谢幕：接下来4次普攻攻速设为1并获得无限射程，普攻替换为加农炮击；第4炮额外造成144%伤害。\nBase Damage: 155 / 235 / 350\nAbility Scaling: 15 / 22 / 34"}
        ,{HeroType::Rumble, "Rumble", 1, 1, 700, 700, 45, 1, 110, 40, 40, 40, 0.60, 0.25, UnitPositionType::Frontline, {Origin::Yordle}, {Role::Guardian}, "机械重组：获得持续4秒的护盾，并发射火焰，在一个锥形内造成魔法伤害。\nShield: 350 / 430 / 550\nDamage: 180% / 270% / 405% Armor"}
        ,{HeroType::Sona, "Sona", 1, 1, 500, 500, 20, 4, 30, 0, 15, 15, 0.70, 0.25, UnitPositionType::Backline, {Origin::Demacia}, {Role::Oracle}, "能量和弦：对2个附近敌人造成魔法伤害，同时为生命值最低的友军回复生命值。\nDamage: 130 / 195 / 300\nHealing: 40 / 50 / 80"}
        ,{HeroType::Ashe, "Ashe", 1, 2, 550, 550, 60, 4, 80, 20, 20, 20, 0.75, 0.25, UnitPositionType::Backline, {Origin::Freljord}, {Role::Swiftshot}, "臻冰之箭：对目标造成物理伤害，并对半径1格圆形区域内的邻格敌人造成伤害；低于30%最大生命值的目标改为受到真实伤害。命中的敌人被施加持续3秒的30%冰冷效果。\nDamage: 135 / 195 / 300 AD + 20 / 30 / 40 AP\nArea Damage: 33% skill damage"}
        ,{HeroType::ChoGath, "Cho'Gath", 1, 2, 850, 850, 50, 1, 100, 40, 45, 45, 0.60, 0.25, UnitPositionType::Frontline, {Origin::Void}, {Role::Dominator}, "碎裂：获得10永久最大生命值，回复生命值，并碎裂目标脚下半径2格的范围。短暂延迟后，范围内敌人被击飞1.5秒并受到魔法伤害。\nHealing: 200 / 225 / 400\nDamage: 5% max health + 45 / 75 / 110 AP"}
        ,{HeroType::XinZhao, "Xin Zhao", 1, 2, 850, 850, 50, 1, 70, 15, 45, 45, 0.65, 0.25, UnitPositionType::Frontline, {Origin::Demacia, Origin::Ionia}, {Role::Protector}, "三重爪击：打击目标3次。每次打击造成物理伤害并回复生命值，最后一击造成1.5秒晕眩。\nDamage: 65 / 100 / 150\nHealing: 105 / 145 / 200"}
        ,{HeroType::Yasuo, "Yasuo", 1, 2, 750, 750, 45, 1, 30, 0, 45, 45, 0.80, 0.25, UnitPositionType::Frontline, {Origin::Ionia}, {Role::Executioner}, "踏前斩：冲刺，随后对邻格敌人造成物理伤害。如果仅有一个敌人被命中，则造成双倍伤害。\nDamage: 95 / 145 / 215 AD + 8 / 12 / 18 AP\nBonus: single-target hit deals double damage"}
        ,{HeroType::Ahri, "Ahri", 1, 3, 650, 650, 30, 4, 30, 0, 25, 25, 0.80, 0.25, UnitPositionType::Backline, {Origin::Ionia}, {Role::Mage}, "狐火：向目标投出3团狐火，每团狐火造成魔法伤害。每第3次施放，投出9团狐火，在目标和至多2名附近敌人之间分摊；如果附近有敌人，则冲刺远离。\nDamage: 82 / 125 / 225 AP per foxfire"}
        ,{HeroType::Jinx, "Jinx", 1, 3, 650, 650, 50, 4, 0, 0, 25, 25, 0.75, 0.25, UnitPositionType::Backline, {Origin::Zaun}, {Role::Gunner}, "枪炮交响曲：被动。攻击指定次数后切换至鱼骨头；鱼骨头状态下，攻击替换为3枚导弹，导弹瞄准随机敌人并造成物理伤害。\nAttack Count: 18 / 18 / 16\nDamage: 54 / 82 / 150 AD + 4 / 6 / 9 AP"}
        ,{HeroType::Loris, "Loris", 1, 3, 950, 950, 60, 1, 140, 70, 50, 50, 0.60, 0.25, UnitPositionType::Frontline, {Origin::Piltover}, {Role::Protector}, "皮城争斗：获得持续4秒的护盾，冲锋并击退目标，对目标和命中的单位造成1.25秒晕眩和魔法伤害。冲锋后，迫使附近敌人瞄准洛里斯。\nShield: 700 / 800 / 1000\nDamage: 150 / 225 / 360"}
        ,{HeroType::Sejuani, "Sejuani", 1, 3, 900, 900, 60, 1, 80, 40, 50, 50, 0.60, 0.25, UnitPositionType::Frontline, {Origin::Freljord}, {Role::Guardian}, "凛冬之怒：获得持续4秒的护盾，打击一个锥形和一条直线。被命中的敌人受到魔法伤害和持续4秒的30%冰冷；如果目标已被冰冷，则晕眩1秒。\nShield: 525 / 575 / 775\nDamage: 70 / 105 / 170"}
    };

    return kShopPool;
}

Unit* createHeroUnit(HeroType heroType, const QString& fallbackName = QStringLiteral("Unit"))
{
    switch (heroType) {
    case HeroType::JarvanIV: return new JarvanIV;
    case HeroType::Jhin: return new Jhin;
    case HeroType::Rumble: return new Rumble;
    case HeroType::Sona: return new Sona;
    case HeroType::Ashe: return new Ashe;
    case HeroType::ChoGath: return new ChoGath;
    case HeroType::XinZhao: return new XinZhao;
    case HeroType::Yasuo: return new Yasuo;
    case HeroType::Ahri: return new Ahri;
    case HeroType::Jinx: return new Jinx;
    case HeroType::Loris: return new Loris;
    case HeroType::Sejuani: return new Sejuani;
    }
    return new Unit(fallbackName);
}

const ShopSlot* randomCostOneShopSlot()
{
    // 每局开局只允许发 1 费英雄，所以先从完整商店池中过滤出 1 费候选。
    std::vector<const ShopSlot*> costOneSlots;
    for (const ShopSlot& slot : shopPool()) {
        if (slot.cost == 1) {
            costOneSlots.push_back(&slot);
        }
    }

    if (costOneSlots.empty()) {
        return nullptr;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(costOneSlots.size()) - 1);
    return costOneSlots[static_cast<std::size_t>(dist(rng))];
}

    //连败理赔金币
int lossStreakCompensation(int lossStreak)
{
    if (lossStreak < 2) {
        return 0;
    }
    if (lossStreak == 2) {
        return 2;
    }
    if (lossStreak == 3) {
        return 4;
    }
    return 8;
}

    //利息机制
int interestForGold(int gold)
{
    constexpr int kInterestCap = 5;
    return qBound(0, gold / 10, kInterestCap);
}

bool hasOrigin(const Unit* unit, Origin wanted)
{
    if (unit == nullptr) {
        return false;
    }
    const std::vector<Origin>& origins = unit->origins();
    return std::find(origins.cbegin(), origins.cend(), wanted) != origins.cend();
}

bool hasRole(const Unit* unit, Role wanted)
{
    if (unit == nullptr) {
        return false;
    }
    const std::vector<Role>& roles = unit->roles();
    return std::find(roles.cbegin(), roles.cend(), wanted) != roles.cend();
}

} // end anonymous namespace

int Game::starredValue(const std::array<int, 3>& values, int star)
{
    const int index = qBound(1, star, 3) - 1;
    return values[static_cast<std::size_t>(index)];
}

Game::Game(QObject* parent)
    : QObject(parent)
    , m_scene(new QGraphicsScene(this))
    , m_combatTimer(new QTimer(this))
    , m_projectileTimer(new QTimer(this))
    , m_preparationTimer(new QTimer(this))
    , m_sellZoneItem(nullptr)
    , m_sellZoneText(nullptr)
    , m_dragActive(false)
    , m_activeUnitId(-1)
    , m_sourceGrid(-1, -1)
    , m_sourceBenchSlot(-1)
    , m_rows(Board::ROWS)
    , m_cols(Board::COLS)
    , m_radius(50.0)
    , m_rowSpacing(75.0)
    , m_nextEquipmentId(0)
    , m_playerInterestGoldSnapshot(0)
    , m_enemyInterestGoldSnapshot(0)
    , m_enemyPreparationDone(false)
    , m_gameOver(false)
    , m_preparationRemainingSeconds(kPreparationDurationSeconds)
    , m_phase(GamePhase::Preparation)
{
    m_combatTimer->setInterval(kCombatTickMs);
    connect(m_combatTimer, &QTimer::timeout, this, [this]() {
        combatTick();
    });
    m_projectileTimer->setInterval(kProjectileTickMs);
    connect(m_projectileTimer, &QTimer::timeout, this, [this]() {
        updateAttackProjectiles();
    });
    m_preparationTimer->setInterval(1000);
    connect(m_preparationTimer, &QTimer::timeout, this, [this]() {
        if (m_phase != GamePhase::Preparation || m_gameOver) {
            return;
        }
        if (m_preparationRemainingSeconds > 0) {
            --m_preparationRemainingSeconds;
        }
        if (m_preparationRemainingSeconds <= 0) {
            beginCombat(true);
            return;
        }
        emit stateChanged();
    });
}

Game::~Game()
{
    qDeleteAll(m_units);
    m_units.clear();
}

void Game::initialize()
{
    buildScene();
    reset();
}

void Game::reset()
{
    m_board.clear();
    m_bench.clear();
    m_enemyBench.clear();
    m_player.reset();
    m_enemy.reset();
    m_enemy.setGold(3);
    m_unitItemById.clear();
    m_equipmentItemById.clear();
    resetCombatState();
    m_combatUnits.clear();
    m_preCombatBoardUnits.clear();
    m_preCombatPositions.clear();
    m_combatStates.clear();
    m_playerCombatSynergies.clear();
    m_enemyCombatSynergies.clear();
    m_enemyShopSlots.clear();
    m_equipmentInventory.clear();
    m_nextEquipmentId = 0;
    m_playerInterestGoldSnapshot = 0;
    m_enemyInterestGoldSnapshot = 0;
    m_enemyPreparationDone = false;
    m_gameOver = false;
    m_phase = GamePhase::Preparation;
    resetPreparationTimer();

    for (UnitItem* item : m_unitItems) {
        if (m_scene != nullptr) {
            m_scene->removeItem(item);
        }
        delete item;
    }
    m_unitItems.clear();

    for (EquipmentItem* item : m_equipmentItems) {
        if (m_scene != nullptr) {
            m_scene->removeItem(item);
        }
        delete item;
    }
    m_equipmentItems.clear();

    qDeleteAll(m_units);
    m_units.clear();

    const ShopSlot* starterSlot = randomCostOneShopSlot();
    if (starterSlot != nullptr) {
        // 随机 1 费英雄会作为开局单位进入备战席，并创建对应的可拖拽图形 item。
        Unit* starterUnit = createUnitFromShopSlot(*starterSlot);
        if (starterUnit != nullptr && m_bench.addUnit(starterUnit)) {
            m_units.append(starterUnit);
            createUnitItem(starterUnit);
        } else {
            delete starterUnit;
        }
    }

    const ShopSlot* enemyStarterSlot = randomCostOneShopSlot();
    if (enemyStarterSlot != nullptr) {
        Unit* enemyStarterUnit = createUnitFromShopSlot(*enemyStarterSlot);
        if (enemyStarterUnit != nullptr) {
            enemyStarterUnit->setOwner(Owner::EnemyCtrl);
            if (m_enemyBench.addUnit(enemyStarterUnit)) {
                m_units.append(enemyStarterUnit);
                createUnitItem(enemyStarterUnit);
            } else {
                delete enemyStarterUnit;
            }
        }
    }

    rollShop();
    rollShopFor(m_enemyShopSlots);
    runEnemyPrepareAgent();
    startPreparationTimer();
    syncFromState();
    emit stateChanged();
}

bool Game::saveGame(const QString& filePath, QString* errorMessage) const
{
    QJsonObject root;
    root["version"] = 2;
    root["phase"] = static_cast<int>(m_phase);
    root["gameOver"] = m_gameOver;
    root["enemyPreparationDone"] = m_enemyPreparationDone;
    root["preparationRemainingSeconds"] = m_preparationRemainingSeconds;
    root["nextEquipmentId"] = m_nextEquipmentId;
    root["playerInterestGoldSnapshot"] = m_playerInterestGoldSnapshot;
    root["enemyInterestGoldSnapshot"] = m_enemyInterestGoldSnapshot;
    root["playerCombatSynergies"] = QJsonArray::fromStringList(m_playerCombatSynergies);
    root["enemyCombatSynergies"] = QJsonArray::fromStringList(m_enemyCombatSynergies);

    auto savePlayer = [](const Player& player) {
        QJsonObject object;
        object["hp"] = player.hp();
        object["gold"] = player.gold();
        object["level"] = player.level();
        object["round"] = player.round();
        object["levelProgress"] = player.levelProgress();
        object["lossStreak"] = player.lossStreak();
        return object;
    };
    root["player"] = savePlayer(m_player);
    root["enemy"] = savePlayer(m_enemy);

    auto saveShop = [](const std::vector<ShopSlot>& shopEntries) {
        QJsonArray array;
        for (const ShopSlot& slot : shopEntries) {
            array.append(static_cast<int>(slot.heroType));
        }
        return array;
    };
    root["shop"] = saveShop(m_shopSlots);
    root["enemyShop"] = saveShop(m_enemyShopSlots);

    QJsonArray inventory;
    for (const InventoryEquipment& equipment : m_equipmentInventory) {
        QJsonObject object;
        object["id"] = equipment.id;
        object["type"] = static_cast<int>(equipment.type);
        inventory.append(object);
    }
    root["equipmentInventory"] = inventory;

    QJsonArray units;
    for (Unit* unit : m_units) {
        if (unit == nullptr) {
            continue;
        }
        QJsonObject object;
        object["name"] = unit->name();
        object["heroType"] = unit->heroType();
        object["star"] = unit->star();
        object["cost"] = unit->cost();
        object["maxHp"] = unit->maxHp();
        object["hp"] = unit->hp();
        object["atk"] = unit->atk();
        object["range"] = unit->range();
        object["maxMana"] = unit->maxMana();
        object["mana"] = unit->mana();
        object["abilityPower"] = unit->abilityPower();
        object["armor"] = unit->armor();
        object["magicResist"] = unit->magicResist();
        object["attackSpeed"] = unit->attackSpeed();
        object["critRate"] = unit->critRate();
        object["owner"] = static_cast<int>(unit->owner());
        object["positionType"] = static_cast<int>(unit->positionType());
        object["state"] = static_cast<int>(unit->state());
        object["combatCopy"] = m_combatUnits.contains(unit);

        QJsonArray origins;
        for (const Origin& origin : unit->origins()) {
            origins.append(static_cast<int>(origin));
        }
        object["origins"] = origins;
        QJsonArray roles;
        for (const Role& role : unit->roles()) {
            roles.append(static_cast<int>(role));
        }
        object["roles"] = roles;
        QJsonArray equipment;
        for (const EquipmentType& item : unit->equipment()) {
            equipment.append(static_cast<int>(item));
        }
        object["equipment"] = equipment;

        const int playerBenchSlot = m_bench.findUnit(unit);
        const int enemyBenchSlot = m_enemyBench.findUnit(unit);
        const auto preparedPosition = m_preCombatPositions.find(unit->id());
        QPoint position = unit->position();
        if (playerBenchSlot >= 0) {
            object["location"] = "playerBench";
            object["slot"] = playerBenchSlot;
        } else if (enemyBenchSlot >= 0) {
            object["location"] = "enemyBench";
            object["slot"] = enemyBenchSlot;
        } else if (isUnitOnBoard(unit)) {
            object["location"] = "board";
            object["x"] = position.x();
            object["y"] = position.y();
        } else if (preparedPosition != m_preCombatPositions.end()) {
            object["location"] = "preparedBoard";
            object["x"] = preparedPosition->second.x();
            object["y"] = preparedPosition->second.y();
        } else {
            object["location"] = "none";
        }

        const auto cooldown = m_attackCooldowns.find(unit->id());
        const auto sinceHit = m_secondsSinceHit.find(unit->id());
        const auto healingTime = m_warmogsHealTime.find(unit->id());
        object["attackCooldown"] = cooldown == m_attackCooldowns.end() ? 0.0 : cooldown->second;
        object["secondsSinceHit"] = sinceHit == m_secondsSinceHit.end() ? 0.0 : sinceHit->second;
        object["warmogsHealTime"] = healingTime == m_warmogsHealTime.end() ? 0.0 : healingTime->second;
        const CombatUnitState* status = combatState(unit);
        object["shield"] = status == nullptr ? 0 : status->shield;
        object["shieldSeconds"] = status == nullptr ? 0.0 : status->shieldSeconds;
        object["stunSeconds"] = status == nullptr ? 0.0 : status->stunSeconds;
        object["chillSeconds"] = status == nullptr ? 0.0 : status->chillSeconds;
        object["attackSpeedBonusMultiplier"] = status == nullptr ? 1.0 : status->attackSpeedBonusMultiplier;
        object["attackSpeedBonusSeconds"] = status == nullptr ? 0.0 : status->attackSpeedBonusSeconds;
        object["basicAttackCount"] = status == nullptr ? 0 : status->basicAttackCount;
        object["skillCastCount"] = status == nullptr ? 0 : status->skillCastCount;
        object["empoweredShots"] = status == nullptr ? 0 : status->empoweredShots;
        units.append(object);
    }
    root["units"] = units;

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = "无法写入存档文件。";
        }
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = "存档写入失败。";
        }
        return false;
    }
    return true;
}

bool Game::loadGame(const QString& filePath, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = "无法打开存档文件。";
        }
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage != nullptr) {
            *errorMessage = "存档格式损坏或不是有效的 JSON 文件。";
        }
        return false;
    }
    const QJsonObject root = document.object();
    const int version = root["version"].toInt();
    if (version != 1 && version != 2) {
        if (errorMessage != nullptr) {
            *errorMessage = "不支持的存档版本。";
        }
        return false;
    }

    resetCombatState();
    m_dragActive = false;
    m_activeUnitId = -1;
    m_sourceGrid = QPoint(-1, -1);
    m_sourceBenchSlot = -1;
    m_board.clear();
    m_bench.clear();
    m_enemyBench.clear();
    m_unitItemById.clear();
    m_equipmentItemById.clear();
    m_combatUnits.clear();
    m_preCombatBoardUnits.clear();
    m_preCombatPositions.clear();
    m_combatStates.clear();
    m_playerCombatSynergies.clear();
    m_enemyCombatSynergies.clear();
    for (UnitItem* item : m_unitItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_unitItems.clear();
    for (EquipmentItem* item : m_equipmentItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_equipmentItems.clear();
    qDeleteAll(m_units);
    m_units.clear();
    m_equipmentInventory.clear();
    m_shopSlots.clear();
    m_enemyShopSlots.clear();

    auto loadPlayer = [](const QJsonObject& object, Player& player) {
        player.restoreState(object["hp"].toInt(),
                            object["gold"].toInt(),
                            object["level"].toInt(),
                            object["round"].toInt(),
                            object["levelProgress"].toInt(),
                            object["lossStreak"].toInt());
    };
    loadPlayer(root["player"].toObject(), m_player);
    loadPlayer(root["enemy"].toObject(), m_enemy);
    m_phase = root["phase"].toInt() == static_cast<int>(GamePhase::Combat)
        ? GamePhase::Combat : GamePhase::Preparation;
    m_gameOver = root["gameOver"].toBool(false);
    m_enemyPreparationDone = root["enemyPreparationDone"].toBool(true);
    m_preparationRemainingSeconds = root["preparationRemainingSeconds"].toInt(kPreparationDurationSeconds);
    m_nextEquipmentId = root["nextEquipmentId"].toInt(0);
    m_playerInterestGoldSnapshot = root["playerInterestGoldSnapshot"].toInt(0);
    m_enemyInterestGoldSnapshot = root["enemyInterestGoldSnapshot"].toInt(0);
    const QJsonArray playerCombatSynergies = root["playerCombatSynergies"].toArray();
    for (const auto& value : playerCombatSynergies) {
        m_playerCombatSynergies.append(value.toString());
    }
    const QJsonArray enemyCombatSynergies = root["enemyCombatSynergies"].toArray();
    for (const auto& value : enemyCombatSynergies) {
        m_enemyCombatSynergies.append(value.toString());
    }

    auto loadShop = [](const QJsonArray& savedSlots, std::vector<ShopSlot>& shopEntries) {
        for (const auto& value : savedSlots) {
            const auto heroType = static_cast<HeroType>(value.toInt());
            for (const ShopSlot& available : shopPool()) {
                if (available.heroType == heroType) {
                    shopEntries.push_back(available);
                    break;
                }
            }
        }
    };
    loadShop(root["shop"].toArray(), m_shopSlots);
    loadShop(root["enemyShop"].toArray(), m_enemyShopSlots);

    const QJsonArray equipmentInventory = root["equipmentInventory"].toArray();
    for (const auto& value : equipmentInventory) {
        const QJsonObject object = value.toObject();
        const InventoryEquipment equipment{object["id"].toInt(),
                                           static_cast<EquipmentType>(object["type"].toInt())};
        m_equipmentInventory.push_back(equipment);
        createEquipmentItem(equipment);
    }

    const QJsonArray savedUnits = root["units"].toArray();
    for (const auto& value : savedUnits) {
        const QJsonObject object = value.toObject();
        const QString unitName = object["name"].toString("Unit");
        int savedHeroType = object["heroType"].toInt(-1);
        if (savedHeroType < 0) {
            for (const ShopSlot& available : shopPool()) {
                if (available.heroName == unitName) {
                    savedHeroType = static_cast<int>(available.heroType);
                    break;
                }
            }
        }
        Unit* unit = savedHeroType >= 0
            ? createHeroUnit(static_cast<HeroType>(savedHeroType), unitName)
            : new Unit(unitName);
        unit->setHeroType(savedHeroType);
        unit->setStar(object["star"].toInt(1));
        unit->setCost(object["cost"].toInt(1));
        unit->setMaxHp(object["maxHp"].toInt(300));
        unit->setHp(object["hp"].toInt(unit->maxHp()));
        unit->setAtk(object["atk"].toInt(30));
        unit->setRange(object["range"].toInt(1));
        unit->setMaxMana(object["maxMana"].toInt(60));
        unit->setMana(object["mana"].toInt(0));
        unit->setAbilityPower(object["abilityPower"].toInt(0));
        unit->setArmor(object["armor"].toInt(30));
        unit->setMagicResist(object["magicResist"].toInt(30));
        unit->setAttackSpeed(object["attackSpeed"].toDouble(0.75));
        unit->setCritRate(object["critRate"].toDouble(0.25));
        unit->setOwner(static_cast<Owner>(object["owner"].toInt()));
        unit->setPositionType(static_cast<UnitPositionType>(object["positionType"].toInt()));
        unit->setState(static_cast<UnitState>(object["state"].toInt()));
        std::vector<Origin> origins;
        const QJsonArray savedOrigins = object["origins"].toArray();
        for (const auto& origin : savedOrigins) {
            origins.push_back(static_cast<Origin>(origin.toInt()));
        }
        unit->setOrigins(origins);
        std::vector<Role> roles;
        const QJsonArray savedRoles = object["roles"].toArray();
        for (const auto& role : savedRoles) {
            roles.push_back(static_cast<Role>(role.toInt()));
        }
        unit->setRoles(roles);
        std::vector<EquipmentType> equipment;
        const QJsonArray savedEquipment = object["equipment"].toArray();
        for (const auto& item : savedEquipment) {
            equipment.push_back(static_cast<EquipmentType>(item.toInt()));
        }
        unit->setEquipment(equipment);

        const QString location = object["location"].toString();
        const QPoint position(object["x"].toInt(-1), object["y"].toInt(-1));
        bool restored = false;
        if (location == "playerBench") {
            restored = m_bench.placeUnit(unit, object["slot"].toInt(-1));
        } else if (location == "enemyBench") {
            restored = m_enemyBench.placeUnit(unit, object["slot"].toInt(-1));
        } else if (location == "board") {
            if (Board::isValidPosition(position) && !m_board.hasUnitAt(position)) {
                m_board.addUnit(unit, position);
                restored = true;
            }
        } else if (location == "preparedBoard") {
            m_preCombatBoardUnits.append(unit);
            m_preCombatPositions[unit->id()] = position;
            restored = true;
        }

        if (!restored) {
            delete unit;
            continue;
        }

        if (object["combatCopy"].toBool(false)) {
            m_combatUnits.append(unit);
            CombatUnitState status;
            status.shield = object["shield"].toInt(0);
            status.shieldSeconds = object["shieldSeconds"].toDouble(0.0);
            status.stunSeconds = object["stunSeconds"].toDouble(0.0);
            status.chillSeconds = object["chillSeconds"].toDouble(0.0);
            status.attackSpeedBonusMultiplier = object["attackSpeedBonusMultiplier"].toDouble(1.0);
            status.attackSpeedBonusSeconds = object["attackSpeedBonusSeconds"].toDouble(0.0);
            status.basicAttackCount = object["basicAttackCount"].toInt(0);
            status.skillCastCount = object["skillCastCount"].toInt(0);
            status.empoweredShots = object["empoweredShots"].toInt(0);
            m_combatStates[unit->id()] = status;
        }
        m_attackCooldowns[unit->id()] = object["attackCooldown"].toDouble(0.0);
        m_secondsSinceHit[unit->id()] = object["secondsSinceHit"].toDouble(0.0);
        m_warmogsHealTime[unit->id()] = object["warmogsHealTime"].toDouble(0.0);
        m_units.append(unit);
        createUnitItem(unit);
    }

    syncFromState();
    if (m_phase == GamePhase::Combat && !m_gameOver) {
        m_combatTimer->start();
    }
    emit stateChanged();
    return true;
}

bool Game::buyLevelProgress()
{
    if (m_gameOver) {
        return false;
    }
    const bool bought = m_player.buyLevelProgress();
    if (bought) {
        emit stateChanged();
    }
    return bought;
}

void Game::handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF&)
{
    Unit* unit = findUnitById(unitId);
    if (unit == nullptr || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)) {
        return;
    }

    m_dragActive = true;
    m_activeUnitId = unitId;
    m_sourceGrid = sourceGrid;
    m_sourceBenchSlot = -1;

    m_sourceBenchSlot = m_bench.findUnit(unit);

    if (UnitItem* item = findUnitItem(unitId)) {
        item->setZValue(kZDraggingUnit);
    }
}

void Game::handleDragMoved(int unitId, const QPoint&, const QPointF& scenePos) const
{
    if (!m_dragActive) {
        return;
    }

    clearGridHighlights();
    clearBenchHighlights();

    if (UnitItem* item = findUnitItem(unitId)) {
        item->setPos(scenePos);
    }

    const QPoint target = worldToGrid(scenePos);
    if (GridItem* targetItem = findGridItem(target)) {
        targetItem->setHoverActive(true);

        if (canApplyDrop(unitId, m_sourceGrid, target)
            || canSwapBoardUnits(unitId, m_sourceGrid, target)
            || canSwapBenchWithBoardUnit(unitId, target)) {
            targetItem->setDropActive(true);
        }
    }

    // Bench slots are plain rect items, so we highlight them here instead of using GridItem state.
    const int benchSlot = findBenchSlot(scenePos, m_benchSlotItems);
    if (benchSlot >= 0 && benchSlot < static_cast<int>(m_benchSlotItems.size())) {
        m_benchSlotItems[benchSlot]->setBrush((canApplyBenchDrop(unitId, benchSlot)
            || canSwapBenchUnits(unitId, benchSlot)
            || canSwapBoardWithBenchUnit(unitId, benchSlot))
            ? QColor(75, 105, 75)
            : QColor(75, 55, 55));
    }

    if (m_sellZoneItem != nullptr) {
        m_sellZoneItem->setBrush(isInSellZone(scenePos)
            ? QColor(165, 70, 55, 210)
            : QColor(115, 50, 45, 170));
    }
}

void Game::handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos)
{
    if (!m_dragActive) {
        return;
    }

    const QPoint target = worldToGrid(scenePos);
    const int benchSlot = findBenchSlot(scenePos, m_benchSlotItems);

    clearGridHighlights();
    clearBenchHighlights();
    if (isInSellZone(scenePos)) {
        sellUnitById(unitId);
    } else if (benchSlot >= 0 && canSwapBenchUnits(unitId, benchSlot)) {
        swapBenchUnits(unitId, benchSlot);
    } else if (benchSlot >= 0 && canSwapBoardWithBenchUnit(unitId, benchSlot)) {
        swapBoardWithBenchUnit(unitId, benchSlot);
    } else if (canSwapBoardUnits(unitId, sourceGrid, target)) {
        swapBoardUnits(unitId, sourceGrid, target);
    } else if (canSwapBenchWithBoardUnit(unitId, target)) {
        swapBenchWithBoardUnit(unitId, target);
    } else if (benchSlot >= 0 && canApplyBenchDrop(unitId, benchSlot)) {
        applyBenchDrop(unitId, benchSlot);
    } else if (canApplyDrop(unitId, sourceGrid, target)) {
        applyDrop(unitId, target);
    }

    if (UnitItem* item = findUnitItem(m_activeUnitId)) {
        item->setZValue(kZUnit);
    }

    m_dragActive = false;
    m_activeUnitId = -1;
    m_sourceGrid = QPoint(-1, -1);
    m_sourceBenchSlot = -1;

    syncFromState();
    emit stateChanged();
}

void Game::handleEquipmentDragStarted(int itemId, const QPointF&)
{
    auto it = m_equipmentItemById.find(itemId);
    if (it != m_equipmentItemById.end() && it->second != nullptr) {
        it->second->setZValue(kZDraggingUnit + 1.0);
    }
}

void Game::handleEquipmentDragMoved(int itemId, const QPointF& scenePos)
{
    auto it = m_equipmentItemById.find(itemId);
    if (it != m_equipmentItemById.end() && it->second != nullptr) {
        it->second->setPos(scenePos);
    }
}

void Game::handleEquipmentDrop(int itemId, const QPointF& scenePos)
{
    Unit* target = playerUnitAtScenePosition(scenePos);
    if (target != nullptr) {
        equipInventoryItem(itemId, target);
    }
    syncEquipmentItems();
    syncFromState();
    emit stateChanged();
}

Unit* Game::findUnitById(int unitId) const
{
    for (Unit* unit : m_units) {
        if (unit && unit->id() == unitId) {
            return unit;
        }
    }
    return nullptr;
}

Unit* Game::playerUnitAtScenePosition(const QPointF& scenePos) const
{
    for (UnitItem* item : m_unitItems) {
        if (item == nullptr || !item->isVisible() || item->unit() == nullptr) {
            continue;
        }
        Unit* unit = item->unit();
        if (unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)) {
            continue;
        }
        if (item->sceneBoundingRect().contains(scenePos)) {
            return unit;
        }
    }
    return nullptr;
}

GridItem* Game::findGridItem(const QPoint& gridPos) const
{
    for (GridItem* item : m_gridItems) {
        if (item && item->gridPos() == gridPos) {
            return item;
        }
    }
    return nullptr;
}

UnitItem* Game::findUnitItem(int unitId) const
{
    auto it = m_unitItemById.find(unitId);
    if (it == m_unitItemById.end()) {
        return nullptr;
    }
    return it->second;
}

int Game::findBenchSlot(const QPointF& scenePos, const std::vector<QGraphicsRectItem*>& slotItems)
{
    for (std::size_t i = 0; i < slotItems.size(); ++i) {
        const QGraphicsRectItem* item = slotItems[i];
        if (item && item->rect().contains(scenePos)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Game::clearGridHighlights() const
{
    for (GridItem* item : m_gridItems) {
        if (!item) {
            continue;
        }
        item->setHoverActive(false);
        item->setDropActive(false);
    }
}

void Game::clearBenchHighlights() const
{
    for (QGraphicsRectItem* item : m_benchSlotItems) {
        if (item) {
            item->setBrush(QColor(45, 45, 55));
        }
    }

    for (QGraphicsRectItem* item : m_enemyBenchSlotItems) {
        if (item) {
            item->setBrush(QColor(55, 40, 45));
        }
    }
}

bool Game::canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const
{
    if (m_phase == GamePhase::Combat)
    {
        return false;
    }
    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)) {
        return false;
    }

    if (!Board::isValidPosition(target)) {
        return false;
    }

    if (!Board::isPlayerHalf(target)) {
        return false;
    }

    if (m_board.hasUnitAt(target)) {
        return false;
    }

    // A source outside the board means the unit is currently coming from the bench.
    if (Board::isValidPosition(source)) {
        if (!Board::isPlayerHalf(source) || source == target) {
            return false;
        }
        return m_board.getUnitAt(source) == unit;
    }

    return m_bench.findUnit(unit) >= 0 && deployedPlayerUnitCount() < m_player.unitCap();
}

bool Game::canApplyBenchDrop(int unitId, int targetSlot) const
{
    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)
        || !Bench::isValidSlot(targetSlot)) {
        return false;
    }

    const int currentBenchSlot = m_bench.findUnit(unit);
    if (currentBenchSlot == targetSlot) {
        return false;
    }

    if (m_phase == GamePhase::Combat) {
        return currentBenchSlot >= 0 && !m_bench.hasUnitAt(targetSlot);
    }

    // Moving to bench is allowed from either another bench slot or a currently occupied board tile.
    return !m_bench.hasUnitAt(targetSlot)
        && (currentBenchSlot >= 0 || m_board.getUnitAt(unit->position()) == unit);
}

bool Game::canSwapBoardUnits(int unitId, const QPoint& source, const QPoint& target) const
{
    if (m_phase == GamePhase::Combat)
    {
        return false;
    }
    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)
        || !Board::isValidPosition(source) || !Board::isValidPosition(target)) {
        return false;
    }

    if (source == target || !Board::isPlayerHalf(source) || !Board::isPlayerHalf(target)) {
        return false;
    }

    Unit* targetUnit = m_board.getUnitAt(target);
    return m_board.getUnitAt(source) == unit
        && targetUnit
        && targetUnit != unit
        && targetUnit->owner() == unit->owner();
}

bool Game::canSwapBenchUnits(int unitId, int targetSlot) const
{
    Unit* unit = findUnitById(unitId);
    const int sourceSlot = m_bench.findUnit(unit);
    return unit
        && unit->owner() == Owner::PlayerCtrl
        && !m_combatUnits.contains(unit)
        && Bench::isValidSlot(sourceSlot)
        && Bench::isValidSlot(targetSlot)
        && sourceSlot != targetSlot
        && m_bench.hasUnitAt(targetSlot);
}

bool Game::canSwapBenchWithBoardUnit(int unitId, const QPoint& target) const
{
    if (m_phase == GamePhase::Combat)
    {
        return false;
    }
    Unit* unit = findUnitById(unitId);
    const int sourceSlot = m_bench.findUnit(unit);
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)
        || !Bench::isValidSlot(sourceSlot) || !Board::isValidPosition(target)) {
        return false;
    }

    if (!Board::isPlayerHalf(target)) {
        return false;
    }

    Unit* targetUnit = m_board.getUnitAt(target);
    return targetUnit && targetUnit != unit && targetUnit->owner() == unit->owner();
}

bool Game::canSwapBoardWithBenchUnit(int unitId, int targetSlot) const
{
    if (m_phase == GamePhase::Combat)
    {
        return false;
    }
    Unit* unit = findUnitById(unitId);
    if (!unit || unit->owner() != Owner::PlayerCtrl || m_combatUnits.contains(unit)
        || !Bench::isValidSlot(targetSlot) || !m_bench.hasUnitAt(targetSlot)) {
        return false;
    }

    const QPoint source = unit->position();
    Unit* targetUnit = m_bench.getUnitAt(targetSlot);
    return Board::isValidPosition(source)
        && Board::isPlayerHalf(source)
        && m_board.getUnitAt(source) == unit
        && targetUnit
        && targetUnit->owner() == unit->owner();
}

void Game::applyDrop(int unitId, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return;
    }

    m_bench.removeUnit(unit);
    m_board.removeUnit(unit);
    m_board.addUnit(unit, target);
}

void Game::applyBenchDrop(int unitId, int targetSlot)
{
    Unit* unit = findUnitById(unitId);
    if (!unit) {
        return;
    }

    m_board.removeUnit(unit);
    m_bench.removeUnit(unit);
    m_bench.placeUnit(unit, targetSlot);
}

void Game::swapBoardUnits(int unitId, const QPoint& source, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    Unit* targetUnit = m_board.getUnitAt(target);
    if (!unit || !targetUnit) {
        return;
    }

    m_board.removeUnit(unit);
    m_board.removeUnit(targetUnit);
    m_board.addUnit(unit, target);
    m_board.addUnit(targetUnit, source);
}

void Game::swapBenchUnits(int unitId, int targetSlot)
{
    Unit* unit = findUnitById(unitId);
    const int sourceSlot = m_bench.findUnit(unit);
    if (!unit) {
        return;
    }

    m_bench.swapUnits(sourceSlot, targetSlot);
}

void Game::swapBenchWithBoardUnit(int unitId, const QPoint& target)
{
    Unit* unit = findUnitById(unitId);
    const int sourceSlot = m_bench.findUnit(unit);
    Unit* targetUnit = m_board.getUnitAt(target);
    if (!unit || !targetUnit) {
        return;
    }

    m_bench.removeUnit(unit);
    m_board.removeUnit(targetUnit);
    m_board.addUnit(unit, target);
    m_bench.placeUnit(targetUnit, sourceSlot);
}

void Game::swapBoardWithBenchUnit(int unitId, int targetSlot)
{
    Unit* unit = findUnitById(unitId);
    Unit* targetUnit = m_bench.getUnitAt(targetSlot);
    if (!unit || !targetUnit) {
        return;
    }

    const QPoint source = unit->position();
    m_board.removeUnit(unit);
    m_bench.removeUnit(targetUnit);
    m_bench.placeUnit(unit, targetSlot);
    m_board.addUnit(targetUnit, source);
}

void Game::buildScene()
{
    m_scene->clear();
    m_gridItems.clear();
    m_benchSlotItems.clear();
    m_enemyBenchSlotItems.clear();
    m_equipmentSlotItems.clear();
    m_unitItems.clear();
    m_unitItemById.clear();
    m_sellZoneItem = nullptr;
    m_sellZoneText = nullptr;

    QRectF totalBounds;
    bool first = true;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QPolygonF poly = cellHexPolygon(row, col);
            auto* gridItem = new GridItem(row, col, poly);
            gridItem->setZValue(kZGrid);
            gridItem->setBaseColor(row < Board::ROWS / 2 ? QColor(80, 60, 60) : QColor(60, 60, 80));

            m_scene->addItem(gridItem);
            m_gridItems.push_back(gridItem);

            const QRectF bounds = gridItem->boundingRect();
            totalBounds = first ? bounds : totalBounds.united(bounds);
            first = false;
        }
    }

    const QRectF boardBounds = totalBounds;

    for (Unit* unit : std::as_const(m_units)) {
        createUnitItem(unit);
    }

    for (int i = 0; i < Bench::SLOTS; ++i) {
        const QRectF rect = benchSlotRect(i, true);
        auto* slotItem = m_scene->addRect(
            rect,
            QPen(QColor(170, 120, 130), 2),
            QColor(55, 40, 45)
        );
        slotItem->setZValue(kZGrid);
        m_enemyBenchSlotItems.push_back(slotItem);
        totalBounds = totalBounds.united(rect);
    }

    for (int i = 0; i < Bench::SLOTS; ++i) {
        const QRectF rect = benchSlotRect(i, false);
        auto* slotItem = m_scene->addRect(
            rect,
            QPen(QColor(160, 160, 170), 2),
            QColor(45, 45, 55)
        );
        slotItem->setZValue(kZGrid);
        m_benchSlotItems.push_back(slotItem);
        totalBounds = totalBounds.united(rect);
    }

    const qreal equipmentColumnX = boardBounds.left() - kEquipmentColumnGap - kEquipmentSlotWidth;
    const QRectF atlasRect(equipmentColumnX,
                           boardBounds.top(),
                           kEquipmentSlotWidth,
                           kEquipmentSlotHeight);
    auto* atlasSlot = new ClickableRectItem(atlasRect, [this]() {
        emit equipmentAtlasRequested();
    });
    atlasSlot->setPen(QPen(QColor(155, 130, 75), 1.8));
    atlasSlot->setBrush(QColor(38, 41, 53));
    atlasSlot->setZValue(kZGrid);
    m_scene->addItem(atlasSlot);
    totalBounds = totalBounds.united(atlasRect);

    auto* atlasText = m_scene->addText("装备\n图谱");
    QFont atlasFont = atlasText->font();
    atlasFont.setPointSize(12);
    atlasFont.setBold(true);
    atlasText->setFont(atlasFont);
    atlasText->setDefaultTextColor(QColor(240, 211, 122));
    const QRectF atlasTextBounds = atlasText->boundingRect();
    atlasText->setPos(atlasRect.center().x() - atlasTextBounds.width() * 0.5,
                      atlasRect.center().y() - atlasTextBounds.height() * 0.5);
    atlasText->setAcceptedMouseButtons(Qt::NoButton);
    atlasText->setZValue(kZGrid + 0.3);
    totalBounds = totalBounds.united(atlasText->sceneBoundingRect());

    for (int i = 0; i < kEquipmentSlotCount; ++i) {
        // 装备图谱占据装备列最上方；实际掉落装备从第二格开始自上而下排列。
        const QRectF rect(equipmentColumnX,
                          atlasRect.bottom() + kEquipmentSlotGap
                              + i * (kEquipmentSlotHeight + kEquipmentSlotGap),
                          kEquipmentSlotWidth,
                          kEquipmentSlotHeight);
        auto* slot = m_scene->addRect(rect, QPen(QColor(110, 100, 75), 1.5), QColor(39, 40, 45));
        slot->setZValue(kZGrid);
        m_equipmentSlotItems.push_back(slot);
        totalBounds = totalBounds.united(rect);
    }

    constexpr qreal sellZoneGap = 12.0;
    const QRectF firstBenchRect = benchSlotRect(0, false);
    // Sell 区域和 bench 格子保持同尺寸，并和玩家备战席横向对齐，避免额外撑大场景缩放。
    const QRectF sellRect(firstBenchRect.left() - kBenchSlotSize - sellZoneGap,
                          firstBenchRect.top(),
                          kBenchSlotSize,
                          kBenchSlotSize);
    QPainterPath sellPath;
    sellPath.addRect(sellRect);
    m_sellZoneItem = m_scene->addPath(
        sellPath,
        QPen(QColor(205, 120, 105), 2),
        QColor(115, 50, 45, 170)
    );
    m_sellZoneItem->setZValue(kZGrid + 0.2);
    totalBounds = totalBounds.united(sellPath.boundingRect());

    m_sellZoneText = m_scene->addText("Sell");
    QFont sellFont = m_sellZoneText->font();
    sellFont.setPointSize(16);
    sellFont.setBold(true);
    m_sellZoneText->setFont(sellFont);
    m_sellZoneText->setDefaultTextColor(QColor(255, 235, 220));
    const QRectF sellTextBounds = m_sellZoneText->boundingRect();
    m_sellZoneText->setPos(sellRect.center().x() - sellTextBounds.width() * 0.5,
                           sellRect.center().y() - sellTextBounds.height() * 0.5);
    m_sellZoneText->setZValue(kZGrid + 0.3);
    totalBounds = totalBounds.united(m_sellZoneText->sceneBoundingRect());

    m_scene->setSceneRect(totalBounds.adjusted(-4, -16, 20, 20));
}

void Game::syncFromState()
{
    clearGridHighlights();
    clearBenchHighlights();

    // UnitItem rendering is derived from logical state: bench first, then board, otherwise hidden.
    for (UnitItem* item : m_unitItems) {
        if (!item || !item->unit()) {
            continue;
        }

        const int benchSlot = m_bench.findUnit(item->unit());
        if (benchSlot >= 0) {
            item->setVisible(true);
            item->setGridPos(QPoint(-1, -1));
            item->setPos(benchSlotCenter(benchSlot, false));
            item->setZValue(kZUnit);
            item->update();
            continue;
        }

        const int enemyBenchSlot = m_enemyBench.findUnit(item->unit());
        if (enemyBenchSlot >= 0) {
            item->setVisible(true);
            item->setGridPos(QPoint(-1, -1));
            item->setPos(benchSlotCenter(enemyBenchSlot, true));
            item->setZValue(kZUnit);
            item->update();
            continue;
        }

        const QPoint pos = item->unit()->position();
        if (Board::isValidPosition(pos) && m_board.getUnitAt(pos) == item->unit()) {
            item->setVisible(true);
            item->setGridPos(pos);
            item->setPos(gridToWorld(pos.y(), pos.x()));
            item->setZValue(kZUnit);
            item->update();
            continue;
        }

        item->setVisible(false);
        item->update();
    }
    syncEquipmentItems();
}

QPointF Game::gridToWorld(int row, int col) const
{
    const qreal colSpacing = m_radius * qSqrt(3.0);
    const qreal xOffset = (row % 2 == 0) ? colSpacing * 0.5 : 0.0;
    const qreal x = xOffset + col * colSpacing;
    const qreal y = row * m_rowSpacing;
    return {x, y};
}

QPoint Game::worldToGrid(const QPointF& world) const
{
    QPoint best(-1, -1);
    qreal bestDist = 1e18;

    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            const QPointF center = gridToWorld(row, col);
            const qreal dx = world.x() - center.x();
            const qreal dy = world.y() - center.y();
            const qreal d2 = dx * dx + dy * dy;
            if (d2 < bestDist) {
                bestDist = d2;
                best = QPoint(col, row);
            }
        }
    }

    if (bestDist > m_radius * m_radius) {
        return {-1, -1};
    }

    return best;
}

QRectF Game::benchSlotRect(int slot, bool enemyBench) const
{
    QRectF boardBounds;
    bool first = true;
    for (int row = 0; row < Board::ROWS; ++row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QRectF cellBounds = cellHexPolygon(row, col).boundingRect();
            boardBounds = first ? cellBounds : boardBounds.united(cellBounds);
            first = false;
        }
    }

    constexpr qreal benchWidth = Bench::SLOTS * kBenchSlotSize + (Bench::SLOTS - 1) * kBenchSlotGap;
    const qreal startX = boardBounds.center().x() - benchWidth * 0.5;
    const qreal x = startX + slot * (kBenchSlotSize + kBenchSlotGap);
    const qreal y = enemyBench
        ? boardBounds.top() - kBenchRowGap - kBenchSlotSize
        : boardBounds.bottom() + kBenchRowGap;
    return {x, y, kBenchSlotSize, kBenchSlotSize};
}

QPointF Game::benchSlotCenter(int slot, bool enemyBench) const
{
    return benchSlotRect(slot, enemyBench).center();
}

bool Game::isInSellZone(const QPointF& scenePos) const
{
    return m_sellZoneItem != nullptr && m_sellZoneItem->path().contains(scenePos);
}

QPolygonF Game::cellHexPolygon(int row, int col) const
{
    const QPointF center = gridToWorld(row, col);
    QPolygonF poly;
    poly.reserve(6);

    for (int i = 0; i < 6; ++i) {
        const qreal angleDeg = 60.0 * i - 90.0;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        poly.append(QPointF(
            center.x() + m_radius * qCos(angleRad),
            center.y() + m_radius * qSin(angleRad)
        ));
    }

    return poly;
}


int Game::deployedPlayerUnitCount() const
{
    return static_cast<int>(deployedUnits(Owner::PlayerCtrl).size());
}

int Game::benchPlayerUnitCount() const
{
    int count = 0;
    for (int i = 0; i < Bench::SLOTS; ++i)
    {
        if (m_bench.hasUnitAt(i)){count++;}
    }
    return count;
}

bool Game::benchFull() const
{
    return benchPlayerUnitCount() >= Bench::SLOTS;
}

void Game::createUnitItem(Unit* unit)
{
    if (unit == nullptr || m_scene == nullptr)return;

    auto* unitItem = new UnitItem(unit);
    m_scene->addItem(unitItem);

    m_unitItems.push_back(unitItem);
    m_unitItemById[unit->id()] = unitItem;

    connect(unitItem, &UnitItem::dragStarted,this, &Game::handleDragStarted);

    connect(unitItem, &UnitItem::dragMoved,this, &Game::handleDragMoved);

    connect(unitItem, &UnitItem::dragDropped,this, &Game::handleDropCommand);
    connect(unitItem, &UnitItem::detailRequested, this, [this](int unitId) {
        if (Unit* unit = findUnitById(unitId)) {
            emit unitCardRequested(unit);
        }
    });
}

void Game::createEquipmentItem(const InventoryEquipment& equipment)
{
    if (m_scene == nullptr) {
        return;
    }
    auto* item = new EquipmentItem(equipment.id,
                                   equipmentShortName(equipment.type),
                                   equipmentStatText(equipment.type),
                                   equipmentColor(equipment.type));
    m_scene->addItem(item);
    m_equipmentItems.push_back(item);
    m_equipmentItemById[equipment.id] = item;
    connect(item, &EquipmentItem::dragStarted, this, &Game::handleEquipmentDragStarted);
    connect(item, &EquipmentItem::dragMoved, this, &Game::handleEquipmentDragMoved);
    connect(item, &EquipmentItem::dragDropped, this, &Game::handleEquipmentDrop);
}

void Game::syncEquipmentItems()
{
    for (std::size_t index = 0; index < m_equipmentInventory.size(); ++index) {
        const int id = m_equipmentInventory[index].id;
        auto it = m_equipmentItemById.find(id);
        if (it == m_equipmentItemById.end() || it->second == nullptr
            || index >= m_equipmentSlotItems.size()) {
            continue;
        }
        EquipmentItem* item = it->second;
        const QRectF rect = m_equipmentSlotItems[index]->rect();
        item->setPos(rect.center());
        item->setZValue(kZUnit);
        item->setVisible(true);
    }
}

void Game::rollShop()
{
    rollShopFor(m_shopSlots);
}

void Game::rollShopFor(std::vector<ShopSlot>& targetSlots)
{
    const std::vector<ShopSlot>& availableShopSlots = shopPool();
    if (availableShopSlots.empty())return;
    targetSlots.clear();
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(availableShopSlots.size()) - 1);
    constexpr int kShopSlotCount = 5;

    for (int i = 0; i < kShopSlotCount; ++i)
    {
        const int slotIndex = dist(rng);
        targetSlots.push_back(availableShopSlots[static_cast<std::size_t>(slotIndex)]);
    }
}

bool Game::refreshShop()
{
    if (m_gameOver) {
        return false;
    }
    constexpr int kShopRefreshCost = 4;
    if (m_player.gold() < kShopRefreshCost)return false;

    m_player.spendGold(kShopRefreshCost);
    rollShop();

    emit stateChanged();
    return true;
}

bool Game::playerOwnsHero(const QString& heroName) const
{
    return std::any_of(m_units.cbegin(), m_units.cend(), [&heroName](const Unit* unit) {
        return unit != nullptr && unit->owner() == Owner::PlayerCtrl && unit->name() == heroName;
    });
}

bool Game::buyShopUnit(int slotIndex)
{
    if (m_gameOver) return false;
    if (slotIndex < 0 || benchFull())return false;
    const auto shopIndex = static_cast<std::size_t>(slotIndex);
    if (shopIndex >= m_shopSlots.size())return false;

    const ShopSlot slot = m_shopSlots[shopIndex];
    const int cost = slot.cost;
    if (cost > m_player.gold())return false;
    auto* unit = createUnitFromShopSlot(slot);
    if (unit == nullptr)return false;
    if (m_bench.addUnit(unit) == false)
    {
        delete unit;
        return false;
    }
    m_units.append(unit);
    createUnitItem(unit);
    m_shopSlots.erase(m_shopSlots.begin() + static_cast<std::vector<ShopSlot>::difference_type>(shopIndex));
    m_player.spendGold(cost);
    syncFromState();
    emit stateChanged();
    while (unit != nullptr)
    {
        unit = tryMergeUnit(unit);
    }
    return true;
}

Unit* Game::createUnitFromShopSlot(const ShopSlot& slot)
{
    Unit* unit = createHeroUnit(slot.heroType, slot.heroName);
    if (unit != nullptr) {
        unit->setHeroType(static_cast<int>(slot.heroType));
        unit->setPositionType(slot.positionType);
    }
    return unit;
}

QList<Unit*> Game::findMergeCandidates(const Unit* baseUnit) const
{
    if (baseUnit == nullptr)return {};

    QList<Unit*> candidates;
    for (Unit* unit : m_units)
    {
        if (unit == nullptr || unit == baseUnit) {
            continue;
        }
        if (m_phase == GamePhase::Combat
            && m_preCombatPositions.find(unit->id()) != m_preCombatPositions.end()) {
            continue;
        }
        if (unit->name() == baseUnit->name()
            && unit->owner() == baseUnit->owner()
            && unit->star() == baseUnit->star())
        {
            candidates.append(unit);
        }
    }
    candidates.append(const_cast<Unit*>(baseUnit));
    return candidates;
}

void Game::removeUnitCompletely(Unit* unit)
{
    if (unit == nullptr)return;

    m_board.removeUnit(unit);
    m_bench.removeUnit(unit);
    m_enemyBench.removeUnit(unit);
    m_combatUnits.removeOne(unit);
    m_attackCooldowns.erase(unit->id());
    m_secondsSinceHit.erase(unit->id());
    m_warmogsHealTime.erase(unit->id());
    m_combatStates.erase(unit->id());

    auto itemIt = m_unitItemById.find(unit->id());
    if (itemIt != m_unitItemById.end())
    {
        UnitItem* item = itemIt->second;
        if (m_scene)
        {
            m_scene->removeItem(item);
        }
        m_unitItems.erase(
            std::remove(m_unitItems.begin(), m_unitItems.end(), item),
            m_unitItems.end()
            );
        m_unitItemById.erase(itemIt);
        delete item;
    }
    m_units.removeOne(unit);
    delete unit;
}

Unit* Game::tryMergeUnit(const Unit* unit)
{
    if (unit == nullptr)return nullptr;

    const QList<Unit*> candidates = findMergeCandidates(unit);
    if (candidates.size() < 3)return nullptr;


    QList<Unit*> onBoardCandidates;
    QList<Unit*> onBenchCandidates;
    for (Unit* candidate : candidates)
    {
        const QPoint pos = candidate->position();

        if (Board::isValidPosition(pos) && m_board.getUnitAt(pos) == candidate)
        {
            onBoardCandidates.append(candidate);
        }else if (m_bench.findUnit(candidate) != -1 || m_enemyBench.findUnit(candidate) != -1)
        {
            onBenchCandidates.append(candidate);
        }
    }

    QList<Unit*> finalThreeCandidates;
    for (Unit* candidate : onBoardCandidates)
    {
        if (finalThreeCandidates.size() >= 3)break;

        finalThreeCandidates.append(candidate);
    }
    for (Unit* candidate : onBenchCandidates)
    {
        if (finalThreeCandidates.size() >= 3)break;

        finalThreeCandidates.append(candidate);
    }

    Unit* first = finalThreeCandidates[0];
    Unit* second = finalThreeCandidates[1];
    Unit* third = finalThreeCandidates[2];

    removeUnitCompletely(second);
    removeUnitCompletely(third);

    first->setCost(first->cost() * 3);
    first->setStar(first->star() + 1);
    first->setMaxHp(first->maxHp() * 2);
    first->setHp(first->maxHp());
    first->setAtk(first->atk() * 2);
    first->setArmor(first->armor() * 2);
    first->setMagicResist(first->magicResist() * 2);
    first->setAttackSpeed(first->attackSpeed() * 1.5);
    first->setCritRate(first->critRate() * 1.5);
    syncFromState();
    emit stateChanged();
    return first;
}

bool Game::sellUnit(Unit* unit)
{
    if (m_gameOver)return false;
    if (unit == nullptr)return false;
    if (m_phase == GamePhase::Combat
        && (isUnitOnBoard(unit) || m_preCombatPositions.find(unit->id()) != m_preCombatPositions.end()))return false;
    if (unit->owner() != Owner::PlayerCtrl)return false;
    int proceeds = unit->cost();
    if (proceeds <= 0)return false;

    m_player.addGold(proceeds);
    removeUnitCompletely(unit);
    syncFromState();
    emit stateChanged();
    return true;
}

bool Game::sellUnitById(int unitId)
{
    Unit* unit = findUnitById(unitId);
    if (unit == nullptr)return false;
    return sellUnit(unit);
}

void Game::applyEquipmentStats(Unit* unit, EquipmentType type)
{
    if (unit == nullptr) {
        return;
    }
    switch (type) {
    case EquipmentType::BFSword:
        unit->setAtk(unit->atk() + 10);
        break;
    case EquipmentType::RecurveBow:
        unit->setAttackSpeed(unit->attackSpeed() * 1.10);
        break;
    case EquipmentType::NeedlesslyLargeRod:
        unit->setAbilityPower(unit->abilityPower() + 10);
        break;
    case EquipmentType::TearOfTheGoddess:
        unit->setMana(qMin(unit->maxMana(), unit->mana() + 15));
        break;
    case EquipmentType::ChainVest:
        unit->setMaxHp(unit->maxHp() + 150);
        unit->setHp(unit->hp() + 150);
        break;
    case EquipmentType::NegatronCloak:
        unit->setMagicResist(unit->magicResist() + 20);
        break;
    case EquipmentType::GiantsBelt:
        unit->setMaxHp(unit->maxHp() + 150);
        unit->setHp(unit->hp() + 150);
        break;
    case EquipmentType::SparringGloves:
        unit->setCritRate(unit->critRate() + 0.20);
        break;
    case EquipmentType::IronSword:
        unit->setAtk(unit->atk() + 15);
        break;
    case EquipmentType::HasteGloves:
        unit->setAttackSpeed(unit->attackSpeed() * 1.20);
        break;
    case EquipmentType::BlueCrystal: {
        // 蓝水晶降低施法阈值；当前法力同步压到新上限以内。
        if (unit->maxMana() <= 0) {
            break;
        }
        const int newMaxMana = qMax(1, unit->maxMana() - 30);
        unit->setMaxMana(newMaxMana);
        unit->setMana(qMin(unit->mana(), newMaxMana));
        break;
    }
    default:
        break;
    }
}

bool Game::equipInventoryItem(int itemId, Unit* unit)
{
    if (unit == nullptr || unit->owner() != Owner::PlayerCtrl || m_gameOver) {
        return false;
    }
    auto inventoryIt = std::find_if(m_equipmentInventory.begin(), m_equipmentInventory.end(),
        [itemId](const InventoryEquipment& equipment) { return equipment.id == itemId; });
    if (inventoryIt == m_equipmentInventory.end()) {
        return false;
    }

    const EquipmentType incoming = inventoryIt->type;
    const std::vector<EquipmentType>& equipped = unit->equipment();
    for (int i = 0; i < static_cast<int>(equipped.size()); ++i) {
        EquipmentType combined;
        if (combinedEquipment(equipped[static_cast<std::size_t>(i)], incoming, combined)) {
            // 两件散件的属性已经分别加到英雄身上，合成为成装时只替换标识并启用被动。
            applyEquipmentStats(unit, incoming);
            unit->setEquipmentAt(i, combined);
            m_equipmentInventory.erase(inventoryIt);
            auto graphicsIt = m_equipmentItemById.find(itemId);
            if (graphicsIt != m_equipmentItemById.end()) {
                EquipmentItem* item = graphicsIt->second;
                m_scene->removeItem(item);
                m_equipmentItems.erase(std::remove(m_equipmentItems.begin(), m_equipmentItems.end(), item),
                                       m_equipmentItems.end());
                delete item;
                m_equipmentItemById.erase(graphicsIt);
            }
            return true;
        }
    }

    if (static_cast<int>(equipped.size()) >= unit->equipmentCapacity()) {
        return false;
    }

    applyEquipmentStats(unit, incoming);
    unit->addEquipment(incoming);
    m_equipmentInventory.erase(inventoryIt);
    auto graphicsIt = m_equipmentItemById.find(itemId);
    if (graphicsIt != m_equipmentItemById.end()) {
        EquipmentItem* item = graphicsIt->second;
        m_scene->removeItem(item);
        m_equipmentItems.erase(std::remove(m_equipmentItems.begin(), m_equipmentItems.end(), item),
                               m_equipmentItems.end());
        delete item;
        m_equipmentItemById.erase(graphicsIt);
    }
    return true;
}

void Game::tryDropBasicEquipment()
{
    if (m_equipmentInventory.size() >= static_cast<std::size_t>(kEquipmentSlotCount)) {
        return;
    }
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> chance(1, 100);
    if (chance(rng) > 90) {
        return;
    }

    static constexpr std::array<EquipmentType, 11> basicEquipment = {
        EquipmentType::BFSword,
        EquipmentType::RecurveBow,
        EquipmentType::NeedlesslyLargeRod,
        EquipmentType::TearOfTheGoddess,
        EquipmentType::ChainVest,
        EquipmentType::NegatronCloak,
        EquipmentType::GiantsBelt,
        EquipmentType::SparringGloves,
        EquipmentType::IronSword,
        EquipmentType::HasteGloves,
        EquipmentType::BlueCrystal
    };
    std::uniform_int_distribution<int> itemRoll(0, static_cast<int>(basicEquipment.size()) - 1);
    const InventoryEquipment dropped{m_nextEquipmentId++, basicEquipment[static_cast<std::size_t>(itemRoll(rng))]};
    m_equipmentInventory.push_back(dropped);
    createEquipmentItem(dropped);
    syncEquipmentItems();
}

bool Game::canStartCombat() const
{
    return !m_gameOver
        && m_phase == GamePhase::Preparation
        && m_enemyPreparationDone
        && !deployedUnits(Owner::PlayerCtrl).isEmpty()
        && !deployedUnits(Owner::EnemyCtrl).isEmpty();
}

bool Game::startCombat()
{
    return beginCombat(false);
}

bool Game::endCombat()
{
    if (m_phase == GamePhase::Preparation)
    {
        return false;
    }

    m_phase = GamePhase::Preparation;
    resetCombatState();
    cleanupCombatCopies();
    restorePreparationBoard();
    resetPreparationTimer();
    startPreparationTimer();
    emit stateChanged();
    return true;
}

bool Game::beginCombat(bool forced)
{
    if (m_phase == GamePhase::Combat) {
        return false;
    }

    if (forced) {
        for (int slot = 0; slot < Bench::SLOTS && deployedPlayerUnitCount() < m_player.unitCap(); ++slot) {
            Unit* unit = m_bench.getUnitAt(slot);
            if (unit == nullptr || unit->owner() != Owner::PlayerCtrl) {
                continue;
            }

            for (int row = Board::ROWS / 2; row < Board::ROWS; ++row) {
                bool placed = false;
                for (int col = 0; col < Board::COLS; ++col) {
                    const QPoint target(col, row);
                    if (!m_board.hasUnitAt(target)) {
                        applyDrop(unit->id(), target);
                        placed = true;
                        break;
                    }
                }
                if (placed) {
                    break;
                }
            }
        }
    }

    if (!canStartCombat()) {
        return false;
    }

    // 当前流程由玩家手动开始战斗，此刻锁定金币，战斗中消费不改变本轮利息。
    m_playerInterestGoldSnapshot = m_player.gold();
    m_enemyInterestGoldSnapshot = m_enemy.gold();
    stopPreparationTimer();
    resetCombatState();
    setupCombatCopies();
    m_phase = GamePhase::Combat;
    m_combatTimer->start();
    emit stateChanged();
    return true;
}


bool Game::isUnitOnBoard(const Unit* unit) const
{
    if (unit == nullptr)return false;

    const QPoint pos = unit->position();
    return Board::isValidPosition(pos) && m_board.getUnitAt(pos) == unit;
}

void Game::createEnemyStarterUnit()
{
    const ShopSlot* starterSlot = randomCostOneShopSlot();
    if (starterSlot == nullptr)
    {
        return;
    }
    Unit* enemyUnit = createUnitFromShopSlot(*starterSlot);
    if (enemyUnit == nullptr)
    {
        return;
    }
    enemyUnit->setOwner(Owner::EnemyCtrl);
    constexpr QPoint enemyPos(3,1);
    if (!Board::isValidPosition(enemyPos) || m_board.hasUnitAt(enemyPos))
    {
        delete enemyUnit;
        return;
    }
    m_board.addUnit(enemyUnit, enemyPos);
    m_units.append(enemyUnit);
    createUnitItem(enemyUnit);
}

Unit* Game::cloneUnitForCombat(const Unit* source)
{
    if (source == nullptr) {
        return nullptr;
    }

    const int heroType = source->heroType();
    Unit* clone = heroType >= 0
        ? createHeroUnit(static_cast<HeroType>(heroType), source->name())
        : new Unit(source->name());

    clone->setStar(source->star());
    clone->setCost(source->cost());
    clone->setMaxHp(source->maxHp());
    clone->setHp(source->maxHp());
    clone->setAtk(source->atk());
    clone->setRange(source->range());
    clone->setMaxMana(source->maxMana());
    clone->setMana(source->mana());
    clone->setAbilityPower(source->abilityPower());
    clone->setArmor(source->armor());
    clone->setMagicResist(source->magicResist());
    clone->setAttackSpeed(source->attackSpeed());
    clone->setCritRate(source->critRate());
    clone->setOwner(source->owner());
    clone->setPositionType(source->positionType());
    clone->setHeroType(heroType);
    clone->setEquipment(source->equipment());
    clone->setOrigins(source->origins());
    clone->setRoles(source->roles());
    return clone;
}

void Game::setupCombatCopies()
{
    cleanupCombatCopies();
    m_preCombatBoardUnits.clear();
    m_preCombatPositions.clear();

    QList<Unit*> sources = deployedUnits(Owner::PlayerCtrl);
    sources.append(deployedUnits(Owner::EnemyCtrl));

    for (Unit* source : std::as_const(sources)) {
        if (source == nullptr) {
            continue;
        }
        m_preCombatBoardUnits.append(source);
        m_preCombatPositions[source->id()] = source->position();
    }

    for (Unit* source : std::as_const(m_preCombatBoardUnits)) {
        m_board.removeUnit(source);
    }

    for (Unit* source : std::as_const(m_preCombatBoardUnits)) {
        Unit* clone = cloneUnitForCombat(source);
        if (clone == nullptr) {
            continue;
        }

        const auto posIt = m_preCombatPositions.find(source->id());
        if (posIt == m_preCombatPositions.end() || m_board.hasUnitAt(posIt->second)) {
            delete clone;
            continue;
        }

        m_board.addUnit(clone, posIt->second);
        m_units.append(clone);
        m_combatUnits.append(clone);
        m_combatStates[clone->id()] = CombatUnitState{};
        createUnitItem(clone);
    }

    // 羁绊只在战斗副本生成后施加，真实备战阵容不会积累临时属性。
    applyCombatSynergies();
    syncFromState();
}

void Game::cleanupCombatCopies()
{
    const QList<Unit*> combatUnits = m_combatUnits;
    for (Unit* unit : combatUnits) {
        removeUnitCompletely(unit);
    }
    m_combatUnits.clear();
}

void Game::restorePreparationBoard()
{
    for (Unit* unit : std::as_const(m_preCombatBoardUnits)) {
        if (unit == nullptr) {
            continue;
        }

        const auto posIt = m_preCombatPositions.find(unit->id());
        if (posIt == m_preCombatPositions.end() || m_board.hasUnitAt(posIt->second)) {
            continue;
        }

        unit->setHp(unit->maxHp());
        m_board.addUnit(unit, posIt->second);
    }

    m_preCombatBoardUnits.clear();
    m_preCombatPositions.clear();
    syncFromState();
}

QList<Unit*> Game::deployedUnits(Owner owner) const
{
    QList<Unit*> units;
    for (Unit* unit : m_units)
    {
        if (unit != nullptr && unit->hp() > 0 && isUnitOnBoard(unit) && unit->owner() == owner)
        {
            units.append(unit);
        }
    }
    return units;
}

CombatUnitState& Game::combatState(const Unit* unit)
{
    return m_combatStates[unit->id()];
}

const CombatUnitState* Game::combatState(const Unit* unit) const
{
    if (unit == nullptr) {
        return nullptr;
    }
    const auto it = m_combatStates.find(unit->id());
    return it == m_combatStates.end() ? nullptr : &it->second;
}

QString Game::combatStatusText(const Unit* unit) const
{
    const CombatUnitState* status = combatState(unit);
    if (status == nullptr) {
        return {};
    }
    QStringList entries;
    if (status->shield > 0) {
        entries << QString("护盾 %1").arg(status->shield);
    }
    if (status->stunSeconds > 0.0) {
        entries << QString("眩晕 %1 秒").arg(status->stunSeconds, 0, 'f', 1);
    }
    if (status->chillSeconds > 0.0) {
        entries << QString("冰冷 %1 秒").arg(status->chillSeconds, 0, 'f', 1);
    }
    if (status->attackSpeedBonusSeconds > 0.0) {
        entries << QString("攻速增益 %1 秒").arg(status->attackSpeedBonusSeconds, 0, 'f', 1);
    }
    return entries.isEmpty() ? QString() : "战斗状态：" + entries.join(" | ");
}

QStringList Game::activePlayerSynergyTexts()
{
    if (m_phase == GamePhase::Combat) {
        return m_playerCombatSynergies;
    }
    return computeActiveSynergies(Owner::PlayerCtrl, deployedUnits(Owner::PlayerCtrl), false);
}

QStringList Game::computeActiveSynergies(Owner owner, const QList<Unit*>& units, bool applyBonuses)
{
    QStringList descriptions;
    auto countOrigin = [&units](Origin origin) {
        return static_cast<int>(std::count_if(units.cbegin(), units.cend(), [origin](const Unit* unit) {
            return hasOrigin(unit, origin);
        }));
    };
    auto countRole = [&units](Role role) {
        return static_cast<int>(std::count_if(units.cbegin(), units.cend(), [role](const Unit* unit) {
            return hasRole(unit, role);
        }));
    };
    auto eachMatching = [&units](auto predicate, auto bonus) {
        for (Unit* unit : units) {
            if (predicate(unit)) {
                bonus(unit);
            }
        }
    };

    const int demacia = countOrigin(Origin::Demacia);
    const int demaciaHp = demacia >= 3 ? 180 : demacia >= 2 ? 100 : 0;
    if (demaciaHp > 0) {
        descriptions << QString("德玛西亚 %1 - 生命 +%2").arg(demacia).arg(demaciaHp);
        if (applyBonuses) {
            eachMatching([](const Unit* unit) { return hasOrigin(unit, Origin::Demacia); },
                         [demaciaHp](Unit* unit) {
                             unit->setMaxHp(unit->maxHp() + demaciaHp);
                             unit->setHp(unit->hp() + demaciaHp);
                         });
        }
    }

    const int ionia = countOrigin(Origin::Ionia);
    const double ioniaSpeed = ionia >= 3 ? 1.20 : ionia >= 2 ? 1.10 : 1.0;
    if (ioniaSpeed > 1.0) {
        descriptions << QString("艾欧尼亚 %1 - 攻速 +%2%").arg(ionia).arg(qRound((ioniaSpeed - 1.0) * 100));
        if (applyBonuses) {
            eachMatching([](const Unit* unit) { return hasOrigin(unit, Origin::Ionia); },
                         [ioniaSpeed](Unit* unit) { unit->setAttackSpeed(unit->attackSpeed() * ioniaSpeed); });
        }
    }

    const int freljord = countOrigin(Origin::Freljord);
    if (freljord >= 2) {
        descriptions << QString("弗雷尔卓德 %1 - 双抗 +20").arg(freljord);
        if (applyBonuses) {
            eachMatching([](const Unit* unit) { return hasOrigin(unit, Origin::Freljord); },
                         [](Unit* unit) {
                             unit->setArmor(unit->armor() + 20);
                             unit->setMagicResist(unit->magicResist() + 20);
                         });
        }
    }

    const int guardian = countRole(Role::Guardian);
    const int guardianShield = guardian >= 3 ? 220 : guardian >= 2 ? 120 : 0;
    if (guardianShield > 0) {
        descriptions << QString("护卫 %1 - 护盾 +%2").arg(guardian).arg(guardianShield);
        if (applyBonuses) {
            eachMatching([](const Unit* unit) { return hasRole(unit, Role::Guardian); },
                         [this, guardianShield](const Unit* unit) { combatState(unit).shield += guardianShield; });
        }
    }

    const int protector = countRole(Role::Protector);
    if (protector >= 2) {
        descriptions << QString("神盾使 %1 - 护盾 +180").arg(protector);
        if (applyBonuses) {
            eachMatching([](const Unit* unit) { return hasRole(unit, Role::Protector); },
                         [this](const Unit* unit) { combatState(unit).shield += 180; });
        }
    }

    const int gunner = countRole(Role::Gunner);
    if (gunner >= 2) {
        descriptions << QString("枪手 %1 - 35% 概率追加一次普攻").arg(gunner);
    }
    Q_UNUSED(owner);
    return descriptions;
}

void Game::applyCombatSynergies()
{
    m_playerCombatSynergies = computeActiveSynergies(Owner::PlayerCtrl, deployedUnits(Owner::PlayerCtrl), true);
    m_enemyCombatSynergies = computeActiveSynergies(Owner::EnemyCtrl, deployedUnits(Owner::EnemyCtrl), true);
}

Unit* Game::findNearestEnemy(const Unit* attacker) const
{
    if (attacker == nullptr)
    {
        return nullptr;
    }
    Owner enemyOwner;
    if (attacker->owner() == Owner::PlayerCtrl)
    {
        enemyOwner = Owner::EnemyCtrl;
    }else
    {
        enemyOwner = Owner::PlayerCtrl;
    }
    Unit* bestTarget = nullptr;

    QList<Unit*> enemies = deployedUnits(enemyOwner);
    qint64 bestDistance = std::numeric_limits<qint64>::max();
    for (Unit* enemy : std::as_const(enemies))
    {
        if (enemy == nullptr)
        {
            continue;
        }

        const QPoint attackerPos = attacker->position();
        const QPoint enemyPos = enemy->position();
        const qint64 distance = euclideanDistanceSquared(attackerPos, enemyPos);
        const QPoint selectedPos = bestTarget == nullptr ? QPoint() : bestTarget->position();
        const bool preferredTieBreak = bestTarget == nullptr
            || enemy->hp() > bestTarget->hp()
            || (enemy->hp() == bestTarget->hp() && enemyPos.x() < selectedPos.x())
            || (enemy->hp() == bestTarget->hp() && enemyPos.x() == selectedPos.x()
                && enemyPos.y() > selectedPos.y());
        if (distance < bestDistance
            || (distance == bestDistance && preferredTieBreak)) {
            bestDistance = distance;
            bestTarget = enemy;
        }
    }

    return bestTarget;
}

bool Game::isInAttackRange(const Unit* attacker, const Unit* target) const
{
    if (attacker == nullptr || target == nullptr) {
        return false;
    }

    if (!isUnitOnBoard(attacker) || !isUnitOnBoard(target)) {
        return false;
    }

    const QPoint attackerPos = attacker->position();
    const QPoint targetPos = target->position();

    const CombatUnitState* status = combatState(attacker);
    const bool jhinEmpowered = attacker->heroType() == static_cast<int>(HeroType::Jhin)
        && status != nullptr && status->empoweredShots > 0;
    const int range = jhinEmpowered ? Board::ROWS + Board::COLS : attacker->range();
    return hexGridDistance(attackerPos, targetPos) <= range;
}

void Game::dealDamage(Unit* target, int damage)
{
    if (target == nullptr || damage <= 0) {
        return;
    }
    CombatUnitState& status = combatState(target);
    const int absorbed = qMin(status.shield, damage);
    status.shield -= absorbed;
    const int healthDamage = damage - absorbed;
    // 受到攻击即会打断狂徒恢复，即使本次伤害全部被护盾吸收。
    m_secondsSinceHit[target->id()] = 0.0;
    m_warmogsHealTime[target->id()] = 0.0;
    if (healthDamage > 0) {
        target->setHp(qMax(0, target->hp() - healthDamage));
    }
}

void Game::healUnit(Unit* unit, int amount)
{
    if (unit != nullptr && amount > 0) {
        unit->setHp(qMin(unit->maxHp(), unit->hp() + amount));
    }
}

QList<Unit*> Game::skillAreaTargets(const Unit* target, Owner targetOwner, int maximumTargets) const
{
    QList<Unit*> targets;
    if (target == nullptr) {
        return targets;
    }
    targets.append(const_cast<Unit*>(target));
    const QPoint center = target->position();
    const QList<Unit*> candidates = deployedUnits(targetOwner);
    for (Unit* candidate : candidates) {
        if (candidate == nullptr || candidate == target) {
            continue;
        }
        if (hexGridDistance(candidate->position(), center) <= 1) {
            targets.append(candidate);
            if (maximumTargets > 0 && targets.size() >= maximumTargets) {
                break;
            }
        }
    }
    return targets;
}

void Game::castSkill(Unit* caster, Unit* target)
{
    if (caster == nullptr || target == nullptr || caster->maxMana() <= 0) {
        return;
    }
    // 通过虚函数多态分发到具体英雄类的实现，替代原来的 switch。
    // 通用前置处理（mana 清零、施法计数）已移入各英雄类实现中。
    caster->castSkill(this, target);
    showAttackProjectile(caster, target);
    syncFromState();
    emit stateChanged();
}

double Game::effectiveAttackSpeed(const Unit* unit) const
{
    if (unit == nullptr) {
        return 0.1;
    }
    const CombatUnitState* status = combatState(unit);
    double speed = unit->attackSpeed();
    if (unit->heroType() == static_cast<int>(HeroType::Jhin) && status != nullptr && status->empoweredShots > 0) {
        speed = 1.0;
    }
    if (status != nullptr) {
        if (status->attackSpeedBonusSeconds > 0.0) {
            speed *= status->attackSpeedBonusMultiplier;
        }
        if (status->chillSeconds > 0.0) {
            speed *= 0.70;
        }
    }
    return qMax(0.1, speed);
}

void Game::performAttack(Unit* attacker, Unit* target)
{
    if (attacker == nullptr || target == nullptr) {
        return;
    }

    if (!isUnitOnBoard(attacker) || !isUnitOnBoard(target)) {
        return;
    }

    if (attacker->owner() == target->owner()) {
        return;
    }

    CombatUnitState& attackerState = combatState(attacker);
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> probabilityRoll(0.0, 1.0);
    const int alliedGunnerCount = static_cast<int>(std::count_if(m_combatUnits.cbegin(), m_combatUnits.cend(),
        [this, attacker](const Unit* candidate) {
            return candidate != nullptr
                && candidate->owner() == attacker->owner()
                && candidate->hp() > 0
                && hasRole(candidate, Role::Gunner)
                && this->isUnitOnBoard(candidate);
        }));
    const int totalShots = alliedGunnerCount >= 2 && hasRole(attacker, Role::Gunner)
        && probabilityRoll(rng) < 0.35
        ? 2
        : 1;

    for (int shotIndex = 0; shotIndex < totalShots; ++shotIndex) {
        if (attacker->hp() <= 0 || !isUnitOnBoard(attacker)) {
            break;
        }

        Unit* currentTarget = target;
        if (currentTarget == nullptr || currentTarget->hp() <= 0 || !isUnitOnBoard(currentTarget)) {
            currentTarget = findNearestEnemy(attacker);
        }
        if (currentTarget == nullptr) {
            break;
        }

        const bool critical = probabilityRoll(rng) < qBound(0.0, attacker->critRate(), 1.0);
        int damage = critical ? attacker->atk() * 2 : attacker->atk();
        if (critical && currentTarget->hasEquipment(EquipmentType::Thornmail)) {
            damage = qFloor(damage * 0.90);
        }

        if (attacker->heroType() == static_cast<int>(HeroType::Jhin) && attackerState.empoweredShots > 0) {
            if (attackerState.empoweredShots == 1) {
                damage += Game::starredValue({155, 235, 350}, attacker->star());
            }
            --attackerState.empoweredShots;
        }

        const bool jinxRocket = attacker->heroType() == static_cast<int>(HeroType::Jinx)
            && attackerState.empoweredShots > 0;
        dealDamage(currentTarget, damage);
        if (jinxRocket) {
            const int bonus = Game::starredValue({58, 88, 159}, attacker->star());
            const QList<Unit*> splashTargets = skillAreaTargets(currentTarget, currentTarget->owner(), 3);
            for (Unit* extra : splashTargets) {
                if (extra != currentTarget) {
                    dealDamage(extra, bonus);
                }
            }
            attackerState.empoweredShots = 0;
            attackerState.basicAttackCount = 0;
        } else if (attacker->heroType() == static_cast<int>(HeroType::Jinx)) {
            ++attackerState.basicAttackCount;
            const int threshold = Game::starredValue({18, 18, 16}, attacker->star());
            if (attackerState.basicAttackCount >= threshold) {
                attackerState.empoweredShots = 1;
            }
        }

        if (attacker->maxMana() > 0) {
            attacker->setMana(qMin(attacker->maxMana(), attacker->mana() + 10));
        }

        if (attacker->hasEquipment(EquipmentType::GuinsoosRageblade)) {
            attacker->setAttackSpeed(attacker->attackSpeed() * 1.06);
        }
        if (attacker->hasEquipment(EquipmentType::SpearOfShojin)) {
            attacker->setMana(qMin(attacker->maxMana(), attacker->mana() + 5));
        }
        if (currentTarget->hasEquipment(EquipmentType::Thornmail)) {
            dealDamage(attacker, 25);
        }
        showAttackProjectile(attacker, currentTarget);
    }

    syncFromState();
    emit stateChanged();
}

void Game::updateCombatEffects()
{
    for (Unit* unit : std::as_const(m_combatUnits)) {
        if (unit == nullptr) {
            continue;
        }
        CombatUnitState& state = combatState(unit);
        state.stunSeconds = qMax(0.0, state.stunSeconds - kCombatTickSeconds);
        state.chillSeconds = qMax(0.0, state.chillSeconds - kCombatTickSeconds);
        state.attackSpeedBonusSeconds = qMax(0.0, state.attackSpeedBonusSeconds - kCombatTickSeconds);
        if (state.attackSpeedBonusSeconds <= 0.0) {
            state.attackSpeedBonusMultiplier = 1.0;
        }
        if (state.shield > 0 && state.shieldSeconds > 0.0) {
            state.shieldSeconds = qMax(0.0, state.shieldSeconds - kCombatTickSeconds);
            if (state.shieldSeconds <= 0.0) {
                state.shield = 0;
            }
        }
    }
}

void Game::updateWarmogsHealing()
{
    for (Unit* unit : std::as_const(m_combatUnits)) {
        if (unit == nullptr || unit->hp() <= 0 || !isUnitOnBoard(unit)) {
            continue;
        }
        double& secondsSinceHit = m_secondsSinceHit[unit->id()];
        secondsSinceHit += kCombatTickSeconds;
        if (!unit->hasEquipment(EquipmentType::WarmogsArmor) || secondsSinceHit < 3.0) {
            continue;
        }

        double& healTime = m_warmogsHealTime[unit->id()];
        healTime += kCombatTickSeconds;
        if (healTime >= 1.0 && unit->hp() < unit->maxHp()) {
            const int healing = qCeil(unit->maxHp() * 0.05);
            unit->setHp(qMin(unit->maxHp(), unit->hp() + healing));
            healTime -= 1.0;
        }
    }
}

void Game::removeDeadUnits()
{
    QList<Unit*> deadUnits;

    for (Unit* unit : std::as_const(m_units)) {
        if (unit == nullptr) {
            continue;
        }

        if (!isUnitOnBoard(unit)) {
            continue;
        }

        if (unit->hp() <= 0) {
            deadUnits.append(unit);
        }
    }

    for (Unit* unit : deadUnits) {
        removeUnitCompletely(unit);
    }

    if (!deadUnits.isEmpty()) {
        syncFromState();
        emit stateChanged();
    }
}

void Game::combatTick()
{
    if (m_phase != GamePhase::Combat) {
        return;
    }

    updateCombatEffects();
    QList<Unit*> attackers = deployedUnits(Owner::PlayerCtrl);
    attackers.append(deployedUnits(Owner::EnemyCtrl));

    for (Unit* attacker : std::as_const(attackers)) {
        if (attacker == nullptr) {
            continue;
        }

        if (!isUnitOnBoard(attacker) || attacker->hp() <= 0) {
            attacker->setState(UnitState::Dead);
            continue;
        }

        if (combatState(attacker).stunSeconds > 0.0) {
            attacker->setState(UnitState::Idle);
            continue;
        }
        double& cooldown = m_attackCooldowns[attacker->id()];
        cooldown = qMax(0.0, cooldown - kCombatTickSeconds);
        if (cooldown > 0.0) {
            attacker->setState(UnitState::Idle);
            continue;
        }

        Unit* target = findNearestEnemy(attacker);
        if (target == nullptr) {
            attacker->setState(UnitState::Idle);
            continue;
        }

        if (attacker->maxMana() > 0 && attacker->mana() >= attacker->maxMana()) {
            attacker->setState(UnitState::Casting);
            castSkill(attacker, target);
            cooldown = 1.0 / effectiveAttackSpeed(attacker);
            continue;
        }

        if (isInAttackRange(attacker, target)) {
            attacker->setState(UnitState::Attacking);
            performAttack(attacker, target);
            const double attackSpeed = effectiveAttackSpeed(attacker);
            cooldown = 1.0 / attackSpeed;
        } else {
            attacker->setState(UnitState::Moving);
            moveUnitTowardTarget(attacker, target);
            cooldown = kMoveCooldownSeconds;
        }
    }

    updateWarmogsHealing();
    removeDeadUnits();

    const bool playerAlive = !deployedUnits(Owner::PlayerCtrl).isEmpty();
    const bool enemyAlive = !deployedUnits(Owner::EnemyCtrl).isEmpty();
    if (!enemyAlive) {
        finishCombat(Owner::PlayerCtrl);
    } else if (!playerAlive) {
        finishCombat(Owner::EnemyCtrl);
    }
}

void Game::finishCombat(Owner winner)
{
    if (m_phase != GamePhase::Combat) {
        return;
    }

    const int remainingUnits = static_cast<int>(deployedUnits(winner).size());
    const int damage = qMax(1, remainingUnits);

    m_combatTimer->stop();
    resetCombatState();
    cleanupCombatCopies();
    restorePreparationBoard();
    m_phase = GamePhase::Preparation;
    resetPreparationTimer();

    constexpr int kBaseRoundReward = 4;
    const int roundReward = kBaseRoundReward + qMax(0, m_player.round() - 1);
    const int winnerBonusGold = roundReward;
    const int playerInterest = interestForGold(m_playerInterestGoldSnapshot);
    const int enemyInterest = interestForGold(m_enemyInterestGoldSnapshot);
    const bool playerWon = winner == Owner::PlayerCtrl;
    Player& winningPlayer = playerWon ? m_player : m_enemy;
    Player& losingPlayer = playerWon ? m_enemy : m_player;

    m_player.addGold(roundReward);
    m_enemy.addGold(roundReward);
    m_player.addGold(playerInterest);
    m_enemy.addGold(enemyInterest);
    m_player.addLevelProgress(roundReward);
    m_enemy.addLevelProgress(roundReward);


    winningPlayer.recordWin();
    const int loserLossStreak = losingPlayer.recordLoss();
    const int loserCompensation = lossStreakCompensation(loserLossStreak);
    winningPlayer.addGold(winnerBonusGold);
    losingPlayer.addGold(loserCompensation);
    losingPlayer.addLevelProgress(loserCompensation);

    if (playerWon) {
        tryDropBasicEquipment();
        m_enemy.takeDamage(damage);
    } else {
        m_player.takeDamage(damage);
    }

    if (m_player.isDead() || m_enemy.isDead()) {
        m_gameOver = true;
        stopPreparationTimer();
        syncFromState();
        emit stateChanged();
        emit gameFinished(!m_player.isDead());
        return;
    }

    m_player.advanceRound();
    m_enemy.advanceRound();

    rollShop();
    rollShopFor(m_enemyShopSlots);
    runEnemyPrepareAgent();
    startPreparationTimer();

    syncFromState();
    emit stateChanged();
}

void Game::resetCombatState()
{
    if (m_combatTimer != nullptr) {
        m_combatTimer->stop();
    }
    m_attackCooldowns.clear();
    m_secondsSinceHit.clear();
    m_warmogsHealTime.clear();
    m_combatStates.clear();
    m_playerCombatSynergies.clear();
    m_enemyCombatSynergies.clear();
    clearAttackProjectiles();
}

void Game::showAttackProjectile(const Unit* attacker, const Unit* target)
{
    if (m_scene == nullptr || attacker == nullptr || target == nullptr) {
        return;
    }

    UnitItem* attackerItem = findUnitItem(attacker->id());
    UnitItem* targetItem = findUnitItem(target->id());
    if (attackerItem == nullptr || targetItem == nullptr) {
        return;
    }

    const QPointF start = attackerItem->sceneBoundingRect().center();
    const QPointF end = targetItem->sceneBoundingRect().center();
    const QColor color = attacker->owner() == Owner::PlayerCtrl
        ? QColor(100, 195, 255)
        : QColor(255, 116, 90);

    QPen trailPen(QColor(color.red(), color.green(), color.blue(), 115), 3.0);
    trailPen.setCapStyle(Qt::RoundCap);
    auto* trail = m_scene->addLine(QLineF(start, start), trailPen);
    trail->setZValue(kZDraggingUnit + 1.0);
    auto* bullet = m_scene->addEllipse(-6.0, -6.0, 12.0, 12.0, QPen(color.lighter(130), 1.5), QBrush(color));
    bullet->setPos(start);
    bullet->setZValue(kZDraggingUnit + 1.1);

    // 子弹只负责表现攻击过程，伤害仍在 performAttack 中即时结算，不改变战斗判定。
    m_attackProjectiles.push_back({bullet, trail, start, end, color, 0, kProjectileLifetimeTicks});
    if (!m_projectileTimer->isActive()) {
        m_projectileTimer->start();
    }
}

void Game::updateAttackProjectiles()
{
    auto it = m_attackProjectiles.begin();
    while (it != m_attackProjectiles.end()) {
        ++it->ageTicks;
        const qreal progress = qMin(1.0, static_cast<qreal>(it->ageTicks) / it->lifetimeTicks);
        const QPointF current = it->start + (it->end - it->start) * progress;
        const QPointF trailStart = it->start + (it->end - it->start) * qMax(0.0, progress - 0.16);
        if (it->bullet != nullptr) {
            it->bullet->setPos(current);
        }
        if (it->trail != nullptr) {
            it->trail->setLine(QLineF(trailStart, current));
        }

        if (it->ageTicks >= it->lifetimeTicks) {
            if (m_scene != nullptr) {
                m_scene->removeItem(it->bullet);
                m_scene->removeItem(it->trail);
            }
            delete it->bullet;
            delete it->trail;
            it = m_attackProjectiles.erase(it);
        } else {
            ++it;
        }
    }
    if (m_attackProjectiles.empty() && m_projectileTimer != nullptr) {
        m_projectileTimer->stop();
    }
}

void Game::clearAttackProjectiles()
{
    if (m_projectileTimer != nullptr) {
        m_projectileTimer->stop();
    }
    for (const AttackProjectile& projectile : m_attackProjectiles) {
        if (m_scene != nullptr) {
            m_scene->removeItem(projectile.bullet);
            m_scene->removeItem(projectile.trail);
        }
        delete projectile.bullet;
        delete projectile.trail;
    }
    m_attackProjectiles.clear();
}

void Game::startPreparationTimer()
{
    if (m_preparationTimer != nullptr && !m_gameOver && m_phase == GamePhase::Preparation) {
        m_preparationTimer->start();
    }
}

void Game::stopPreparationTimer()
{
    if (m_preparationTimer != nullptr) {
        m_preparationTimer->stop();
    }
}

void Game::resetPreparationTimer()
{
    m_preparationRemainingSeconds = kPreparationDurationSeconds;
}

void Game::moveUnitTowardTarget(Unit* unit, const Unit* target)
{
    if (unit == nullptr || target == nullptr || !isUnitOnBoard(unit) || !isUnitOnBoard(target)) {
        return;
    }

    const QPoint start = unit->position();
    const QPoint targetPos = target->position();
    const int attackRange = unit->range();

    // 如果已在攻击范围内则无需移动
    const int startDistance = hexGridDistance(start, targetPos);
    if (startDistance <= attackRange) {
        return;
    }

    std::unordered_map<int, QPoint> parent; // 用线性索引记录前驱
    std::deque<QPoint> queue;
    queue.push_back(start);
    parent[gridIndex(start)] = start;

    QPoint bestGoal = start;
    int bestDistance = startDistance;
    bool foundAttackCell = false;

    while (!queue.empty()) {
        const QPoint cur = queue.front();
        queue.pop_front();

        const int distance = hexGridDistance(cur, targetPos);
        const bool inAttackRange = distance <= attackRange;
        if ((inAttackRange && (!foundAttackCell || distance < bestDistance))
            || (!foundAttackCell && distance < bestDistance)) {
            bestDistance = distance;
            bestGoal = cur;
            foundAttackCell = inAttackRange;
            if (foundAttackCell) {
                continue;
            }
        }

        for (const QPoint& offset : hexNeighborOffsets(cur.y())) {
            const QPoint next = cur + offset;
            if (!Board::isValidPosition(next)) continue;
            if (m_board.hasUnitAt(next) && next != start) continue;
            if (parent.count(gridIndex(next))) continue; // 已访问
            parent[gridIndex(next)] = cur;
            queue.push_back(next);
        }
    }

    if (bestGoal == start) {
        return;
    }

    // 回溯路径，取第一步
    QPoint step = bestGoal;
    while (parent[gridIndex(step)] != start) {
        step = parent[gridIndex(step)];
    }

    m_board.removeUnit(unit);
    m_board.addUnit(unit, step);
    syncFromState();
    emit stateChanged();
}

bool Game::enemyBenchFull() const
{
    int count = 0;
    for (int i = 0; i < Bench::SLOTS; ++i) {
        if (m_enemyBench.hasUnitAt(i)) {
            ++count;
        }
    }
    return count >= Bench::SLOTS;
}

int Game::enemyUnitScore(const Unit* unit) const
{
    if (unit == nullptr || unit->owner() != Owner::EnemyCtrl) {
        return -1;
    }

    int matchingPeers = 0;
    for (Unit* candidate : m_units) {
        if (candidate != nullptr && candidate != unit
            && candidate->owner() == Owner::EnemyCtrl
            && candidate->name() == unit->name()
            && candidate->star() == unit->star()) {
            ++matchingPeers;
        }
    }

    return unit->cost() * 220 + unit->star() * 650 + unit->maxHp() / 8
        + unit->atk() * 3 + matchingPeers * 260;
}

int Game::enemyShopSlotScore(const ShopSlot& slot) const
{
    int matchingUnits = 0;
    for (Unit* unit : m_units) {
        if (unit != nullptr
            && unit->owner() == Owner::EnemyCtrl
            && unit->name() == slot.heroName
            && unit->star() == slot.star) {
            ++matchingUnits;
        }
    }

    const int mergeBonus = matchingUnits >= 2 ? 2400 : matchingUnits * 420;
    const int lateRoundBonus = qMax(0, m_enemy.round() - 10) * slot.cost * 22;
    return slot.cost * 220 + slot.star * 650 + slot.maxHp / 8 + slot.atk * 3
        + mergeBonus + lateRoundBonus;
}

Unit* Game::weakestEnemyBenchUnit() const
{
    Unit* weakest = nullptr;
    int weakestScore = std::numeric_limits<int>::max();
    for (int slot = 0; slot < Bench::SLOTS; ++slot) {
        Unit* candidate = m_enemyBench.getUnitAt(slot);
        const int score = enemyUnitScore(candidate);
        if (candidate != nullptr && score < weakestScore) {
            weakest = candidate;
            weakestScore = score;
        }
    }
    return weakest;
}

bool Game::sellEnemyBenchUnit(Unit* unit)
{
    if (unit == nullptr || unit->owner() != Owner::EnemyCtrl || m_enemyBench.findUnit(unit) < 0) {
        return false;
    }

    const int proceeds = unit->cost();
    if (proceeds <= 0) {
        return false;
    }

    m_enemy.addGold(proceeds);
    removeUnitCompletely(unit);
    return true;
}

bool Game::makeRoomForEnemyShopSlot(const ShopSlot& slot)
{
    if (!enemyBenchFull()) {
        return true;
    }

    Unit* weakest = weakestEnemyBenchUnit();
    if (weakest == nullptr) {
        return false;
    }

    const int requiredImprovement = m_enemy.round() >= 12 || m_enemy.gold() >= 40 ? -120 : 100;
    if (enemyShopSlotScore(slot) < enemyUnitScore(weakest) + requiredImprovement) {
        return false;
    }

    // 满席位时只出售备战席中的弱牌，保留已经部署的战斗阵容。
    return sellEnemyBenchUnit(weakest);
}

bool Game::refreshEnemyShop()
{
    constexpr int kShopRefreshCost = 4;
    if (!m_enemy.spendGold(kShopRefreshCost)) {
        return false;
    }

    rollShopFor(m_enemyShopSlots);
    return true;
}

bool Game::buyEnemyShopUnit(int slotIndex)
{
    if (slotIndex < 0) {
        return false;
    }

    const auto shopIndex = static_cast<std::size_t>(slotIndex);
    if (shopIndex >= m_enemyShopSlots.size()) {
        return false;
    }

    const ShopSlot slot = m_enemyShopSlots[shopIndex];
    if (!makeRoomForEnemyShopSlot(slot)) {
        return false;
    }
    if (!m_enemy.spendGold(slot.cost)) {
        return false;
    }

    Unit* unit = createUnitFromShopSlot(slot);
    if (unit == nullptr) {
        m_enemy.addGold(slot.cost);
        return false;
    }

    unit->setOwner(Owner::EnemyCtrl);
    if (!m_enemyBench.addUnit(unit)) {
        delete unit;
        m_enemy.addGold(slot.cost);
        return false;
    }

    m_units.append(unit);
    createUnitItem(unit);
    m_enemyShopSlots.erase(m_enemyShopSlots.begin() + static_cast<std::vector<ShopSlot>::difference_type>(shopIndex));

    while (unit != nullptr) {
        unit = tryMergeUnit(unit);
    }
    return true;
}

QPoint Game::preferredEnemyDeployPosition(const Unit* unit) const
{
    static constexpr std::array<int, Board::COLS> cols = {3, 4, 2, 5, 1, 6, 0, 7};
    static constexpr std::array<int, 2> frontlineRows = {3, 2};
    static constexpr std::array<int, 2> backlineRows = {0, 1};
    const std::array<int, 2>& rows = unit != nullptr && unit->positionType() == UnitPositionType::Backline
        ? backlineRows
        : frontlineRows;

    // 前排和后排分别优先从中央向两侧占满一条横线，兼顾接战效率和防阻塞。
    for (int row : rows) {
        for (int col : cols) {
            const QPoint pos(col, row);
            if (Board::isValidPosition(pos) && !Board::isPlayerHalf(pos) && !m_board.hasUnitAt(pos)) {
                return pos;
            }
        }
    }
    return {-1, -1};
}

void Game::arrangeEnemyFormation()
{
    const QList<Unit*> enemyUnits = deployedUnits(Owner::EnemyCtrl);
    for (Unit* unit : enemyUnits) {
        m_board.removeUnit(unit);
    }

    // 准备结束时重新摆放旧单位，使上一回合残留阵型也遵循前后排规则。
    for (UnitPositionType type : {UnitPositionType::Frontline, UnitPositionType::Backline}) {
        for (Unit* unit : enemyUnits) {
            if (unit == nullptr || unit->positionType() != type) {
                continue;
            }
            QPoint target = preferredEnemyDeployPosition(unit);
            if (!Board::isValidPosition(target)) {
                target = fallbackEnemyDeployPosition();
            }
            if (Board::isValidPosition(target)) {
                m_board.addUnit(unit, target);
            }
        }
    }
}

void Game::optimizeEnemyLineup()
{
    QList<Unit*> enemyUnits;
    for (Unit* unit : m_units) {
        if (unit != nullptr && unit->owner() == Owner::EnemyCtrl) {
            enemyUnits.append(unit);
        }
    }
    if (enemyUnits.isEmpty()) {
        return;
    }

    std::sort(enemyUnits.begin(), enemyUnits.end(), [this](const Unit* lhs, const Unit* rhs) {
        const int lhsScore = enemyUnitScore(lhs);
        const int rhsScore = enemyUnitScore(rhs);
        if (lhsScore != rhsScore) {
            return lhsScore > rhsScore;
        }
        return lhs != nullptr && rhs != nullptr && lhs->id() < rhs->id();
    });

    for (Unit* unit : enemyUnits) {
        m_board.removeUnit(unit);
        m_enemyBench.removeUnit(unit);
        unit->setPosition(QPoint(-1, -1));
    }

    const int deployLimit = qMin(m_enemy.unitCap(), static_cast<int>(enemyUnits.size()));
    for (int index = 0; index < deployLimit; ++index) {
        Unit* unit = enemyUnits[index];
        QPoint target = preferredEnemyDeployPosition(unit);
        if (!Board::isValidPosition(target)) {
            target = fallbackEnemyDeployPosition();
        }
        if (Board::isValidPosition(target)) {
            m_board.addUnit(unit, target);
        }
    }

    QList<Unit*> overflow;
    for (int index = deployLimit; index < enemyUnits.size(); ++index) {
        Unit* unit = enemyUnits[index];
        if (!m_enemyBench.addUnit(unit)) {
            overflow.append(unit);
        }
    }

    for (Unit* unit : overflow) {
        const int proceeds = unit != nullptr ? unit->cost() : 0;
        if (proceeds > 0) {
            m_enemy.addGold(proceeds);
        }
        removeUnitCompletely(unit);
    }
}

QPoint Game::fallbackEnemyDeployPosition() const
{
    for (int row = Board::ROWS / 2 - 1; row >= 0; --row) {
        for (int col = 0; col < Board::COLS; ++col) {
            const QPoint pos(col, row);
            if (!m_board.hasUnitAt(pos)) {
                return pos;
            }
        }
    }
    return {-1, -1};
}

bool Game::deployEnemyUnitFromBench(Unit* unit)
{
    if (unit == nullptr || unit->owner() != Owner::EnemyCtrl) {
        return false;
    }
    if (m_enemyBench.findUnit(unit) < 0) {
        return false;
    }
    if (deployedUnits(Owner::EnemyCtrl).size() >= m_enemy.unitCap()) {
        return false;
    }

    QPoint target = preferredEnemyDeployPosition(unit);
    if (!Board::isValidPosition(target)) {
        target = fallbackEnemyDeployPosition();
    }
    if (!Board::isValidPosition(target)) {
        return false;
    }

    m_enemyBench.removeUnit(unit);
    m_board.addUnit(unit, target);
    return true;
}

void Game::runEnemyPrepareAgent()
{
    m_enemyPreparationDone = false;
    if (m_phase != GamePhase::Preparation) {
        return;
    }

    if (m_enemyShopSlots.empty()) {
        rollShopFor(m_enemyShopSlots);
    }

    while (m_enemy.gold() >= 24 && m_enemy.levelProgressNeeded() > 0) {
        if (!m_enemy.buyLevelProgress()) {
            break;
        }
    }

    optimizeEnemyLineup();

    const bool lateGame = m_enemy.round() >= 12 || m_enemy.level() >= 8;
    const int reserveGold = lateGame ? (m_enemy.gold() >= 60 ? 0 : 4) : 12;
    const int maxOperations = lateGame ? 48 : 18;
    int staleRefreshes = 0;

    for (int pass = 0; pass < maxOperations && m_enemy.gold() > reserveGold; ++pass) {
        bool bought = false;
        std::vector<int> candidateIndexes;
        for (int i = 0; i < static_cast<int>(m_enemyShopSlots.size()); ++i) {
            const ShopSlot& slot = m_enemyShopSlots[static_cast<std::size_t>(i)];
            if (m_enemy.gold() < slot.cost || m_enemy.gold() - slot.cost < reserveGold) {
                continue;
            }
            candidateIndexes.push_back(i);
        }

        std::sort(candidateIndexes.begin(), candidateIndexes.end(), [this](int lhs, int rhs) {
            const ShopSlot& lhsSlot = m_enemyShopSlots[static_cast<std::size_t>(lhs)];
            const ShopSlot& rhsSlot = m_enemyShopSlots[static_cast<std::size_t>(rhs)];
            const int lhsScore = enemyShopSlotScore(lhsSlot);
            const int rhsScore = enemyShopSlotScore(rhsSlot);
            if (lhsScore != rhsScore) {
                return lhsScore > rhsScore;
            }
            return lhs < rhs;
        });

        for (int shopIndex : candidateIndexes) {
            if (buyEnemyShopUnit(shopIndex)) {
                bought = true;
                break;
            }
        }

        if (!bought) {
            ++staleRefreshes;
            if (m_enemy.gold() - 4 >= reserveGold
                && (lateGame || staleRefreshes <= 2)
                && refreshEnemyShop()) {
                continue;
            }
            break;
        }

        staleRefreshes = 0;
        optimizeEnemyLineup();
        if (m_enemyShopSlots.empty() && m_enemy.gold() - 4 >= reserveGold) {
            refreshEnemyShop();
        }
    }

    optimizeEnemyLineup();
    m_enemyPreparationDone = true;
    syncFromState();
    emit stateChanged();
}
