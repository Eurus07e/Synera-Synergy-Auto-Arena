//
// Created by 束宇轩 on 2026/5/20.
//

#include "XinZhao.h"

XinZhao::XinZhao()
    : Unit("Xin Zhao")
{
    m_star = 1;
    m_cost = 2;
    m_maxHp = 850;
    m_hp = 850;
    m_atk = 50;
    m_range = 1;
    m_maxMana = 70;
    m_mana = 15;
    m_armor = 45;
    m_magicResist = 45;
    m_attackSpeed = 0.65;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Demacia, Origin::Ionia };
    m_roles = { Role::Protector };
    m_state = UnitState::Idle;
}

void XinZhao::castSkill()
{
    // 三重爪击：
    // 打击目标3次。
    // 每次打击造成物理伤害并回复生命值。
    // 最后一击造成1.5秒晕眩。
    // 伤害：65/100/150。
    // 治疗：105/145/200。
}
