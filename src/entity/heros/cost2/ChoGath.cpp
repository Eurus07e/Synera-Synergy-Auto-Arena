//
// Created by 束宇轩 on 2026/5/20.
//

#include "ChoGath.h"

ChoGath::ChoGath()
    : Unit("Cho'Gath")
{
    m_star = 1;
    m_cost = 2;
    m_maxHp = 850;
    m_hp = 850;
    m_atk = 50;
    m_range = 1;
    m_maxMana = 100;
    m_mana = 40;
    m_armor = 45;
    m_magicResist = 45;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Void };
    m_roles = { Role::Dominator };
    m_state = UnitState::Idle;
}

void ChoGath::castSkill()
{
    // 碎裂：
    // 获得10永久最大生命值。
    // 回复生命值并碎裂目标脚下半径2格的范围。
    // 短暂延迟后，范围内敌人被击飞1.5秒并受到魔法伤害。
    // 治疗：200/225/400。
    // 伤害：88 = 5%生命上限 + 45法术加成。
    // 法术加成：45/75/110。
}
