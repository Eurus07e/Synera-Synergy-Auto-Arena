//
// Created by 束宇轩 on 2026/5/20.
//

#include "Sona.h"

Sona::Sona()
    : Unit("Sona")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 500;
    m_hp = 500;
    m_atk = 20;
    m_range = 4;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 15;
    m_magicResist = 15;
    m_attackSpeed = 0.70;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Demacia };
    m_roles = { Role::Oracle };
    m_state = UnitState::Idle;
}

void Sona::castSkill()
{
    // 能量和弦：
    // 对2个附近的敌人造成魔法伤害。
    // 同时为生命值最低的友军回复生命值。
    // 伤害：130/195/300。
    // 回复：40/50/80。
}
