#ifndef UNIT_H
#define UNIT_H

#include <QPoint>
#include <QString>
#include <vector>

enum class UnitState
{
    Idle,
    Moving,
    Attacking,
    Casting,
    Dead
};

enum class Owner
{
    PlayerCtrl,
    EnemyCtrl
};

enum class UnitPositionType
{
    Frontline,
    Backline
};

enum class EquipmentType
{
    BFSword,
    RecurveBow,
    NeedlesslyLargeRod,
    TearOfTheGoddess,
    ChainVest,
    NegatronCloak,
    GiantsBelt,
    SparringGloves,
    GuinsoosRageblade,
    SpearOfShojin,
    Thornmail,
    WarmogsArmor,
    // 新装备追加在末尾，保证旧存档中已有装备的数字编号仍可读取。
    IronSword,
    HasteGloves,
    BlueCrystal
};

enum class Origin
{
    Demacia,      // 德玛西亚
    Noxus,        // 诺克萨斯
    Ionia,        // 艾欧尼亚
    Freljord,     // 弗雷尔卓德
    Bilgewater,   // 比尔吉沃特
    Zaun,         // 祖安
    Piltover,     // 皮尔特沃夫
    ShadowIsles,  // 暗影岛
    Targon,       // 巨神峰
    Shurima,      // 恕瑞玛
    Yordle,       // 约德尔人
    Void          // 虚空

};

enum class Role
{
    Warrior,      // 战士
    Mage,         // 法师
    Assassin,     // 刺客
    Ranger,       // 射手
    Vanguard,     // 重装
    Mystic,       // 秘术
    Brawler,      // 斗士
    Protector,    // 神盾使
    Swiftshot,    // 迅捷
    Guardian,     // 护卫
    Executioner,  // 裁决
    Scholar,      // 学者
    Oracle,       // 神谕者
    Gunner,       // 枪手
    Sniper,       // 狙神
    Dominator     // 主宰

};

class Game;

class Unit
{
public:

    explicit Unit(QString name = QString("Unit"));
    virtual ~Unit() = default;

    [[nodiscard]]int id() const { return m_id; }
    [[nodiscard]]QString name() const { return m_name; }
    [[nodiscard]]QPoint position() const { return m_position; }

    void setName(const QString& name) { m_name = name; }
    void setPosition(const QPoint& pos) { m_position = pos; }


    //基础属性的setter
    void setStar(int star) { m_star = star; }
    void setCost(int cost) { m_cost = cost; }
    void setMaxHp(int maxHp) { m_maxHp = maxHp; }
    void setHp(int hp) { m_hp = hp; }
    void setAtk(int atk) { m_atk = atk; }
    void setRange(int range) { m_range = range; }
    void setMaxMana(int maxMana) { m_maxMana = maxMana; }
    void setMana(int mana) { m_mana = mana; }
    void setAbilityPower(int abilityPower) { m_abilityPower = abilityPower; }
    void setArmor(int armor) { m_armor = armor; }
    void setMagicResist(int magicResist) { m_magicResist = magicResist; }
    void setAttackSpeed(double attackSpeed) { m_attackSpeed = attackSpeed; }
    void setCritRate(double critRate) { m_critRate = critRate; }
    void setOwner(Owner owner) { m_owner = owner; }
    void setPositionType(UnitPositionType positionType) { m_positionType = positionType; }
    void addOrigin(Origin origin) { m_origins.push_back(origin); }
    void addRole(Role role) { m_roles.push_back(role); }
    void setState(UnitState state) { m_state = state; }
    void setHeroType(int heroType) { m_heroType = heroType; }
    void addEquipment(EquipmentType equipment) { m_equipment.push_back(equipment); }
    void setEquipmentAt(int index, EquipmentType equipment) { m_equipment[static_cast<std::size_t>(index)] = equipment; }
    void setEquipment(const std::vector<EquipmentType>& equipment) { m_equipment = equipment; }

    //基础属性的getter
    [[nodiscard]]int star() const { return m_star; }
    [[nodiscard]]int cost() const { return m_cost; }
    [[nodiscard]]int maxHp() const { return m_maxHp; }
    [[nodiscard]]int hp() const { return m_hp; }
    [[nodiscard]]int atk() const { return m_atk; }
    [[nodiscard]]int range() const { return m_range; }
    [[nodiscard]]int maxMana() const { return m_maxMana; }
    [[nodiscard]]int mana() const { return m_mana; }
    [[nodiscard]]int abilityPower() const { return m_abilityPower; }
    [[nodiscard]]int armor() const { return m_armor; }
    [[nodiscard]]int magicResist() const { return m_magicResist; }
    [[nodiscard]]double attackSpeed() const { return m_attackSpeed; }
    [[nodiscard]]double critRate() const { return m_critRate; }
    [[nodiscard]]Owner owner() const { return m_owner; }
    [[nodiscard]]UnitPositionType positionType() const { return m_positionType; }
    [[nodiscard]]std::vector<Origin> origins() const { return m_origins; }
    [[nodiscard]]std::vector<Role> roles() const { return m_roles; }
    [[nodiscard]]UnitState state() const { return m_state; }
    [[nodiscard]]int heroType() const { return m_heroType; }
    [[nodiscard]]const std::vector<EquipmentType>& equipment() const { return m_equipment; }
    [[nodiscard]]bool hasEquipment(EquipmentType equipment) const;
    [[nodiscard]]int equipmentCapacity() const { return m_star > 3 ? 3 : m_star; }

    //技能接口
    virtual void castSkill(Game* game, Unit* target) {}

private:
    static int s_nextId;

    int m_id;
    QString m_name;
    QPoint m_position;
protected:
    //基础属性
    int m_star;
    int m_cost;
    int m_maxHp;
    int m_hp;
    int m_atk;
    int m_range;
    int m_maxMana;
    int m_mana;
    int m_abilityPower;
    int m_armor;//护甲
    int m_magicResist;//魔抗
    double m_attackSpeed;
    double m_critRate;//暴击率
    Owner m_owner;
    UnitPositionType m_positionType;
    std::vector<Origin> m_origins;
    std::vector<Role> m_roles;
    std::vector<EquipmentType> m_equipment;
    UnitState m_state;
    // 使用稳定整数标识英雄种类，使读档后的基础 Unit 仍可由 Game 执行对应技能。
    int m_heroType;
};

#endif // UNIT_H
