//
// Created by 束宇轩 on 2026/5/20.
//

#include "ChoGath.h"
#include "core/game.h"

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

void ChoGath::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> area = game->skillAreaTargets(target, enemyOwner, 0);
    Game::healUnit(this, Game::starredValue({200, 225, 400}, star));
    for (Unit* enemy : area) {
        game->dealDamage(enemy, Game::starredValue({88, 118, 155}, star));
    }
}
