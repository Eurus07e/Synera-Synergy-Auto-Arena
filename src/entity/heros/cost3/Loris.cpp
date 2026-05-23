//
// Created by 束宇轩 on 2026/5/20.
//

#include "Loris.h"

Loris::Loris()
    : Unit("Loris")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 950;
    m_hp = 950;
    m_atk = 60;
    m_range = 1;
    m_maxMana = 140;
    m_mana = 70;
    m_armor = 50;
    m_magicResist = 50;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Piltover };
    m_roles = { Role::Protector };
    m_state = UnitState::Idle;
}

void Loris::castSkill()
{
    // 皮城争斗：
    // 获得持续4秒的护盾。
    // 冲锋并击退目标，对目标和命中的单位造成1.25秒晕眩和魔法伤害。
    // 冲锋后，迫使附近的敌人瞄准洛里斯。
    // 护盾值：700/800/1000。
    // 伤害：150/225/360。
}
