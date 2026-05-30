//
// Created by 束宇轩 on 2026/5/20.
//

#include "Ashe.h"
#include "core/game.h"

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

void Ashe::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> area = game->skillAreaTargets(target, enemyOwner, 0);
    game->dealDamage(target, Game::starredValue({155, 225, 340}, star));
    game->combatState(target).chillSeconds = 3.0;
    for (Unit* enemy : area) {
        if (enemy != target) {
            game->dealDamage(enemy, Game::starredValue({51, 74, 112}, star));
            game->combatState(enemy).chillSeconds = 3.0;
        }
    }
}
