//
// Created by 束宇轩 on 2026/5/20.
//

#include "Sona.h"
#include "core/game.h"

Sona::Sona()
    : Unit("Sona")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 500;
    m_hp = 500;
    m_atk = 20;
    m_range = 4;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 15;
    m_magicResist = 15;
    m_attackSpeed = 0.70;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Demacia };
    m_roles = { Role::Oracle };
    m_state = UnitState::Idle;
}

void Sona::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> victims = game->skillAreaTargets(target, enemyOwner, 2);
    for (Unit* enemy : victims) {
        game->dealDamage(enemy, Game::starredValue({130, 195, 300}, star));
    }
    Unit* lowest = nullptr;
    const QList<Unit*> allies = game->deployedUnits(owner());
    for (Unit* ally : allies) {
        if (lowest == nullptr || ally->hp() < lowest->hp()) {
            lowest = ally;
        }
    }
    Game::healUnit(lowest, Game::starredValue({40, 50, 80}, star));
}
