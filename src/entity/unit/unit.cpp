#include "unit.h"
#include <algorithm>
#include <utility>

int Unit::s_nextId = 0;

Unit::Unit(QString name)
    : m_id(s_nextId++)
    , m_name(std::move(name))
    , m_position(0, 0)
    , m_star(1)
    , m_cost(1)
    , m_maxHp(300)
    , m_hp(300)
    , m_atk(30)
    , m_range(1)
    , m_maxMana(60)
    , m_mana(0)
    , m_abilityPower(0)
    , m_armor(30)
    , m_magicResist(30)
    , m_attackSpeed(0.75)
    , m_critRate(0.25)
    , m_owner(Owner::PlayerCtrl)
    , m_positionType(UnitPositionType::Frontline)
    , m_state(UnitState::Idle)
    , m_heroType(-1)
{

}

bool Unit::hasEquipment(EquipmentType equipment) const
{
    //只要容器里有任意一个元素满足条件，就返回 true
    //cbegin() 和 cend() 是只读迭代器
    return std::any_of(m_equipment.cbegin(), m_equipment.cend(),
        [equipment](EquipmentType item) {
            return item == equipment;
        });
}
