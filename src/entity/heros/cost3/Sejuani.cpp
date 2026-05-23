//
// Created by 束宇轩 on 2026/5/20.
//

#include "Sejuani.h"

Sejuani::Sejuani()
    : Unit("Sejuani")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 900;
    m_hp = 900;
    m_atk = 60;
    m_range = 1;
    m_maxMana = 80;
    m_mana = 40;
    m_armor = 50;
    m_magicResist = 50;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Freljord };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

void Sejuani::castSkill()
{
    // 凛冬之怒：
    // 获得持续4秒的护盾。
    // 打击一个锥形和一条直线。
    // 被命中的敌人受到魔法伤害和持续4秒的30%冰冷。
    // 如果目标已被冰冷，则晕眩1秒。
    // 冰冷：降低攻击速度。
    // 护盾：525/575/775。
    // 伤害：70/105/170。
}
