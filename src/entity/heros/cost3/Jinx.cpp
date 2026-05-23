//
// Created by 束宇轩 on 2026/5/20.
//

#include "Jinx.h"

Jinx::Jinx()
    : Unit("Jinx")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 650;
    m_hp = 650;
    m_atk = 50;
    m_range = 4;
    m_maxMana = 0;
    m_mana = 0;
    m_armor = 25;
    m_magicResist = 25;
    m_attackSpeed = 0.75;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Zaun };
    m_roles = { Role::Gunner };
    m_state = UnitState::Idle;
}

void Jinx::castSkill()
{
    // 枪炮交响曲！
    // 被动：攻击18次后，切换至鱼骨头。
    // 在鱼骨头状态下，攻击替换为3枚导弹。
    // 导弹瞄准随机敌人并造成物理伤害。
    // 攻击次数：18/18/16。
    // 伤害：58 = 54物理加成 + 4法术加成。
    // 物理加成：54/82/150。
    // 法术加成：4/6/9。
}
