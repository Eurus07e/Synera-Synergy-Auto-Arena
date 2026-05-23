//
// Created by 束宇轩 on 2026/5/18.
//

#include "Jhin.h"

Jhin::Jhin()
    : Unit("Jhin")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 444;
    m_hp = 444;
    m_atk = 44;
    m_range = 4;
    m_maxMana = 70;
    m_mana = 0;
    m_armor = 30;
    m_magicResist = 30;
    m_attackSpeed = 0.70;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Ionia };
    m_roles = { Role::Gunner, Role::Sniper };
    m_state = UnitState::Idle;
}

void Jhin::castSkill()
{
    // 完美谢幕：
    // 接下来4次普攻，攻速设为1，获得无限射程。
    // 普攻替换为加农炮击，造成物理伤害。
    // 第4炮额外造成144%伤害。
    // 伤害 = 基础伤害 + 法术加成。
    // 基础伤害：155/235/350。
    // 法术加成：15/22/34。
}
