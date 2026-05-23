//
// Created by 束宇轩 on 2026/5/20.
//

#include "JarvanIV.h"

JarvanIV::JarvanIV()
    : Unit("Jarvan IV")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 700;
    m_hp = 700;
    m_atk = 55;
    m_range = 1;
    m_maxMana = 100;
    m_mana = 30;
    m_armor = 40;
    m_magicResist = 40;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Demacia };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

void JarvanIV::castSkill()
{
    // 德邦军旗：
    // 获得持续4秒的护盾。
    // 给全场友军增加持续4秒的攻速。
    // 护盾值：350/425/500。
    // 攻击速度加成：20%/25%/50%。
}
