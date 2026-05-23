#ifndef CORE_GAME_H
#define CORE_GAME_H

#include <QObject>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QPolygonF>
#include <QColor>
#include <array>
#include <unordered_map>
#include <vector>
#include "board.h"
#include "bench.h"
#include "entity/unit/unit.h"
#include "player.h"

class Unit;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsPathItem;
class QGraphicsTextItem;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QTimer;
class GridItem;
class UnitItem;
class EquipmentItem;

enum class GamePhase
{
    Preparation,
    Combat
};

enum class HeroType
{
    JarvanIV,
    Jhin,
    Rumble,
    Sona,
    Ashe,
    ChoGath,
    XinZhao,
    Yasuo,
    Ahri,
    Jinx,
    Loris,
    Sejuani
};

struct ShopSlot
{
    HeroType heroType;
    QString heroName;
    int star;
    int cost;
    int maxHp;
    int hp;
    int atk;
    int range;
    int maxMana;
    int mana;
    int armor;//护甲
    int magicResist;//魔抗
    double attackSpeed;
    double critRate;//暴击率
    UnitPositionType positionType;
    std::vector<Origin> origins;
    std::vector<Role> roles;
    QString skillDescription;
};

struct AttackProjectile
{
    QGraphicsEllipseItem* bullet{};
    QGraphicsLineItem* trail{};
    QPointF start;
    QPointF end;
    QColor color;
    int ageTicks{};
    int lifetimeTicks{};
};

struct InventoryEquipment
{
    int id;
    EquipmentType type;
};

struct CombatUnitState
{
    int shield = 0;
    double stunSeconds = 0.0;
    double chillSeconds = 0.0;
    double attackSpeedBonusMultiplier = 1.0;
    double attackSpeedBonusSeconds = 0.0;
    int basicAttackCount = 0;
    int skillCastCount = 0;
    int empoweredShots = 0;
};

class Game : public QObject
{
    Q_OBJECT

public:
    explicit Game(QObject* parent = nullptr);
    ~Game() override;

    void initialize();
    void reset();
    bool buyLevelProgress();

    [[nodiscard]] QGraphicsScene* scene() const { return m_scene; }
    [[nodiscard]] const Player& player() const { return m_player; }
    [[nodiscard]] const Player& enemy() const { return m_enemy; }
    [[nodiscard]] int deployedPlayerUnitCount() const;
    [[nodiscard]] int benchPlayerUnitCount() const;
    [[nodiscard]] bool benchFull() const;

    void handleDragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleDropCommand(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void handleEquipmentDragStarted(int itemId, const QPointF& scenePos);
    void handleEquipmentDragMoved(int itemId, const QPointF& scenePos);
    void handleEquipmentDrop(int itemId, const QPointF& scenePos);

    [[nodiscard]] const std::vector<ShopSlot>& shopSlots() const { return m_shopSlots; }
    [[nodiscard]] bool playerOwnsHero(const QString& heroName) const;

    bool buyShopUnit(int slotIndex);
    bool refreshShop();
    bool sellUnitById(int unitId);
    bool saveGame(const QString& filePath, QString* errorMessage = nullptr) const;
    bool loadGame(const QString& filePath, QString* errorMessage = nullptr);
    [[nodiscard]] GamePhase phase() const { return m_phase; }
    [[nodiscard]] bool isGameOver() const { return m_gameOver; }
    [[nodiscard]] QStringList activePlayerSynergyTexts();
    [[nodiscard]] QString combatStatusText(const Unit* unit) const;
    [[nodiscard]] bool canStartCombat() const;
    bool startCombat();
    bool endCombat();



    // Combat interface for hero skill polymorphism
    [[nodiscard]] CombatUnitState& combatState(Unit* unit);
    [[nodiscard]] const CombatUnitState* combatState(const Unit* unit) const;
    void dealDamage(Unit* target, int damage);
    static void healUnit(Unit* unit, int amount);
    void showAttackProjectile(const Unit* attacker, const Unit* target);
    [[nodiscard]] QList<Unit*> skillAreaTargets(const Unit* target, Owner targetOwner, int maximumTargets) const;
    [[nodiscard]] QList<Unit*> deployedUnits(Owner owner) const;
    void syncFromState();
    static int starredValue(const std::array<int, 3>& values, int star);


signals:
    void stateChanged();
    void unitCardRequested(Unit* unit);
    void gameFinished(bool playerWon);

private:
    [[nodiscard]] Unit* findUnitById(int unitId) const;
    [[nodiscard]] GridItem* findGridItem(const QPoint& gridPos) const;
    [[nodiscard]] UnitItem* findUnitItem(int unitId) const;
    [[nodiscard]] static int findBenchSlot(const QPointF& scenePos, const std::vector<QGraphicsRectItem*>& slotItems);
    void clearGridHighlights() const;
    void clearBenchHighlights() const;
    [[nodiscard]] bool canApplyDrop(int unitId, const QPoint& source, const QPoint& target) const;
    [[nodiscard]] bool canApplyBenchDrop(int unitId, int targetSlot) const;
    [[nodiscard]] bool canSwapBoardUnits(int unitId, const QPoint& source, const QPoint& target) const;
    [[nodiscard]] bool canSwapBenchUnits(int unitId, int targetSlot) const;
    [[nodiscard]] bool canSwapBenchWithBoardUnit(int unitId, const QPoint& target) const;
    [[nodiscard]] bool canSwapBoardWithBenchUnit(int unitId, int targetSlot) const;
    void applyDrop(int unitId, const QPoint& target);
    void applyBenchDrop(int unitId, int targetSlot);
    void swapBoardUnits(int unitId, const QPoint& source, const QPoint& target);
    void swapBenchUnits(int unitId, int targetSlot);
    void swapBenchWithBoardUnit(int unitId, const QPoint& target);
    void swapBoardWithBenchUnit(int unitId, int targetSlot);
    void buildScene();
    void createUnitItem(Unit* unit);
    void createEquipmentItem(const InventoryEquipment& equipment);
    void syncEquipmentItems();


    [[nodiscard]] QPointF gridToWorld(int row, int col) const;
    [[nodiscard]] QPoint worldToGrid(const QPointF& world) const;
    [[nodiscard]] QPolygonF cellHexPolygon(int row, int col) const;
    [[nodiscard]] QRectF benchSlotRect(int slot, bool enemyBench) const;
    [[nodiscard]] QPointF benchSlotCenter(int slot, bool enemyBench) const;
    [[nodiscard]] bool isInSellZone(const QPointF& scenePos) const;
    [[nodiscard]] bool isUnitOnBoard(const Unit* unit) const;
    [[nodiscard]] Unit* playerUnitAtScenePosition(const QPointF& scenePos) const;

    void rollShop();
    static void rollShopFor(std::vector<ShopSlot>& targetSlots);
    [[nodiscard]] static Unit* createUnitFromShopSlot(const ShopSlot& slot);
    [[nodiscard]] static Unit* cloneUnitForCombat(const Unit* source);
    [[nodiscard]] QList<Unit*> findMergeCandidates(const Unit* baseUnit) const;
    void removeUnitCompletely(Unit* unit);
    Unit* tryMergeUnit(Unit* unit);
    bool sellUnit(Unit* unit);
    bool equipInventoryItem(int itemId, Unit* unit);
    static void applyEquipmentStats(Unit* unit, EquipmentType type);
    void tryDropBasicEquipment();
    [[nodiscard]] static double targetThreatScore(const Unit* attacker, const Unit* target);
    [[nodiscard]]Unit* findNearestEnemy(const Unit* attacker) const;
    [[nodiscard]] bool isInAttackRange(const Unit* attacker, const Unit* target) const;
    void performAttack(Unit* attacker, Unit* target);
    void castSkill(Unit* caster, Unit* target);
    void updateCombatEffects();
    [[nodiscard]] double effectiveAttackSpeed(const Unit* unit) const;
    void applyCombatSynergies();
    [[nodiscard]] QStringList computeActiveSynergies(Owner owner, const QList<Unit*>& units, bool applyBonuses);
    void updateWarmogsHealing();
    void createEnemyStarterUnit();
    void removeDeadUnits();
    void combatTick();
    void finishCombat(Owner winner);
    void resetCombatState();
    void moveUnitTowardTarget(Unit* unit, const Unit* target);
    void setupCombatCopies();
    void cleanupCombatCopies();
    void restorePreparationBoard();
    void runEnemyPrepareAgent();
    bool buyEnemyShopUnit(int slotIndex);
    bool refreshEnemyShop();
    bool deployEnemyUnitFromBench(Unit* unit);
    void arrangeEnemyFormation();
    [[nodiscard]] bool enemyBenchFull() const;
    [[nodiscard]] int enemyUnitScore(const Unit* unit) const;
    [[nodiscard]] int enemyShopSlotScore(const ShopSlot& slot) const;
    [[nodiscard]] Unit* weakestEnemyBenchUnit() const;
    bool sellEnemyBenchUnit(Unit* unit);
    bool makeRoomForEnemyShopSlot(const ShopSlot& slot);
    [[nodiscard]] QPoint preferredEnemyDeployPosition(const Unit* unit) const;
    [[nodiscard]] QPoint fallbackEnemyDeployPosition() const;
    void updateAttackProjectiles();
    void clearAttackProjectiles();


    Board m_board;
    Bench m_bench;
    Bench m_enemyBench;
    QList<Unit*> m_units;
    QList<Unit*> m_combatUnits;
    QList<Unit*> m_preCombatBoardUnits;

    QGraphicsScene* m_scene;
    QTimer* m_combatTimer;
    QTimer* m_projectileTimer;
    std::vector<GridItem*> m_gridItems;
    std::vector<QGraphicsRectItem*> m_benchSlotItems;
    std::vector<QGraphicsRectItem*> m_enemyBenchSlotItems;
    std::vector<UnitItem*> m_unitItems;
    std::vector<QGraphicsRectItem*> m_equipmentSlotItems;
    std::vector<EquipmentItem*> m_equipmentItems;
    std::vector<AttackProjectile> m_attackProjectiles;
    QGraphicsPathItem* m_sellZoneItem;
    QGraphicsTextItem* m_sellZoneText;

    bool m_dragActive;
    int m_activeUnitId;
    QPoint m_sourceGrid;
    int m_sourceBenchSlot;
    std::unordered_map<int, UnitItem*> m_unitItemById;
    std::unordered_map<int, EquipmentItem*> m_equipmentItemById;

    int m_rows;
    int m_cols;
    qreal m_radius;
    qreal m_rowSpacing;
    Player m_player;
    Player m_enemy;
    std::vector<ShopSlot> m_shopSlots;
    std::vector<ShopSlot> m_enemyShopSlots;
    std::vector<InventoryEquipment> m_equipmentInventory;
    int m_nextEquipmentId;
    std::unordered_map<int, double> m_attackCooldowns;
    std::unordered_map<int, double> m_secondsSinceHit;
    std::unordered_map<int, double> m_warmogsHealTime;
    std::unordered_map<int, QPoint> m_preCombatPositions;
    std::unordered_map<int, CombatUnitState> m_combatStates;
    QStringList m_playerCombatSynergies;
    QStringList m_enemyCombatSynergies;
    int m_playerInterestGoldSnapshot;
    int m_enemyInterestGoldSnapshot;
    bool m_enemyPreparationDone;
    bool m_gameOver;

    GamePhase m_phase;

};


#endif // CORE_GAME_H
