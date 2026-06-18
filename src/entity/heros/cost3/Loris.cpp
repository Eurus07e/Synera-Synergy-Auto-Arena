//
// Created by 束宇轩 on 2026/5/20.
//

#include "Loris.h"
#include "core/game.h"

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

// 技能：皮城争斗。洛里斯获得4秒护盾，对目标造成伤害并眩晕1.25秒。
void Loris::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    casterState.shield += Game::starredValue({700, 800, 1000}, star);
    casterState.shieldSeconds = 4.0;
    game->dealDamage(target, Game::starredValue({150, 225, 360}, star));
    game->combatState(target).stunSeconds = 1.25;
}
