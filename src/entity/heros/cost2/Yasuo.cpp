//
// Created by 束宇轩 on 2026/5/20.
//

#include "Yasuo.h"

Yasuo::Yasuo()
    : Unit("Yasuo")
{
    m_star = 1;
    m_cost = 2;
    m_maxHp = 750;
    m_hp = 750;
    m_atk = 45;
    m_range = 1;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 45;
    m_magicResist = 45;
    m_attackSpeed = 0.80;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Ionia };
    m_roles = { Role::Executioner };
    m_state = UnitState::Idle;
}

void Yasuo::castSkill()
{
    // 踏前斩：
    // 冲刺，随后对邻格的敌人们造成物理伤害。
    // 如果仅有一个敌人被命中，则造成双倍伤害。
    // 伤害：103 = 95%物理加成 + 8%法术加成。
    // 基础伤害：95/145/215。
    // 法术加成：8/12/18。
    //
    // 剑刃兄弟：
    // 当亚索和永恩同时登场时，亚索会协助永恩的技能施放。
    // 永恩施放技能时，最强的亚索闪烁至永恩命中的每个目标，
    // 并对每个目标造成100%攻击力物理伤害。
}
