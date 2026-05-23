//
// Created by 束宇轩 on 2026/5/20.
//

#include "Jinx.h"
#include "core/game.h"

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

void Jinx::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    Q_UNUSED(target);
}
