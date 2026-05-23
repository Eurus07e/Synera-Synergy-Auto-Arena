//
// Created by 束宇轩 on 2026/5/20.
//

#include "Rumble.h"

Rumble::Rumble()
    : Unit("Rumble")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 700;
    m_hp = 700;
    m_atk = 45;
    m_range = 1;
    m_maxMana = 110;
    m_mana = 40;
    m_armor = 40;
    m_magicResist = 40;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Yordle };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

void Rumble::castSkill()
{
    // 机械重组：
    // 获得持续4秒的护盾。
    // 发射一阵火焰，在一个锥形内造成魔法伤害。
    // 护盾：350/430/550。
    // 伤害：72护甲加成，对应180%/270%/405%护甲。
}
