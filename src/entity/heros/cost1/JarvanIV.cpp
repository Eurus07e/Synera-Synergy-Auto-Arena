//
// Created by 束宇轩 on 2026/5/20.
//

#include "JarvanIV.h"
#include "core/game.h"

JarvanIV::JarvanIV()
    : Unit("Jarvan IV")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 700;
    m_hp = 700;
    m_atk = 55;
    m_range = 1;
    m_maxMana = 100;
    m_mana = 30;
    m_armor = 40;
    m_magicResist = 40;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Demacia };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

void JarvanIV::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    casterState.shield += game->starredValue({350, 425, 500}, star);
    const double multiplier = star == 1 ? 1.20 : star == 2 ? 1.25 : 1.50;
    for (Unit* ally : game->deployedUnits(owner())) {
        CombatUnitState& state = game->combatState(ally);
        state.attackSpeedBonusMultiplier = qMax(state.attackSpeedBonusMultiplier, multiplier);
        state.attackSpeedBonusSeconds = 4.0;
    }
}
