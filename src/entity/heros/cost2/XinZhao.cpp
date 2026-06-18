//
// Created by 束宇轩 on 2026/5/20.
//

#include "XinZhao.h"
#include "core/game.h"

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

// 技能：三重爪击。对目标造成三段伤害，治疗自身，并眩晕目标1.5秒。
void XinZhao::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    game->dealDamage(target, Game::starredValue({65, 100, 150}, star) * 3);
    Game::healUnit(this, Game::starredValue({105, 145, 200}, star));
    game->combatState(target).stunSeconds = 1.5;
}
