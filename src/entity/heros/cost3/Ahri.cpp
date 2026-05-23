//
// Created by 束宇轩 on 2026/5/18.
//

#include "Ahri.h"

Ahri::Ahri()
    : Unit("Ahri")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 650;
    m_hp = 650;
    m_atk = 30;
    m_range = 4;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 25;
    m_magicResist = 25;
    m_attackSpeed = 0.80;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Ionia };
    m_roles = { Role::Mage };
    m_state = UnitState::Idle;

}

void Ahri::castSkill()
{
    //向目标投出3团狐火，每团狐火造成82(【法术加成】)魔法伤害。
    //每第3次施放，投出9团狐火，在目标和至多2名附近敌人之间分摊。
    //如果附近有敌人，则冲刺远离。
    //伤害：82(【法术加成】) 82/125/225
}
