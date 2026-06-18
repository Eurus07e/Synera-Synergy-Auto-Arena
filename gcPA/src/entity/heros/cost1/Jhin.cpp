//
// Created by 束宇轩 on 2026/5/18.
//

#include "Jhin.h"
#include "core/game.h"

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

void Jhin::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    casterState.empoweredShots = 4;
    Q_UNUSED(target);
}
