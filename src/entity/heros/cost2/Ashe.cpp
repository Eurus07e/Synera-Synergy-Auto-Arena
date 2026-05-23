//
// Created by 束宇轩 on 2026/5/20.
//

#include "Ashe.h"

Ashe::Ashe()
    : Unit("Ashe")
{
    m_star = 1;
    m_cost = 2;
    m_maxHp = 550;
    m_hp = 550;
    m_atk = 60;
    m_range = 4;
    m_maxMana = 80;
    m_mana = 20;
    m_armor = 20;
    m_magicResist = 20;
    m_attackSpeed = 0.75;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Freljord };
    m_roles = { Role::Swiftshot };
    m_state = UnitState::Idle;
}

void Ashe::castSkill()
{
    // 臻冰之箭：
    // 对目标造成物理伤害，并对半径1格圆形区域内的邻格敌人造成伤害。
    // 低于30%最大生命值的目标改为受到真实伤害。
    // 对命中的所有敌人施加持续3秒的30%冰冷效果。
    // 冰冷：降低攻击速度。
    // 伤害：155 = 135物理加成 + 20法术加成。
    // 物理加成：135/195/300。
    // 法术加成：20/30/40。
    // 圆形伤害：51 = 33%技能伤害。
    //
    // 血誓盟约：
    // 当艾希和泰达米尔同时登场时，最强大的己方弈子会在击杀时永久提升强度。
    // 每通过血誓盟约提供10%物理加成给泰达米尔，攻击会发射一支额外箭矢。
    // 额外箭矢造成2物理伤害，即4%攻击力。
}
