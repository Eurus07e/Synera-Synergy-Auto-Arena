//
// Created by 束宇轩 on 2026/5/18.
//

#include "Ahri.h"
#include "core/game.h"

Ahri::Ahri()
    : Unit("Ahri")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 650;
    m_hp = 650;
    m_atk = 30;
    m_range = 4;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 25;
    m_magicResist = 25;
    m_attackSpeed = 0.80;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Ionia };
    m_roles = { Role::Mage };
    m_state = UnitState::Idle;

}

void Ahri::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> victims = game->skillAreaTargets(target, enemyOwner, 3);
    for (Unit* enemy : victims) {
        game->dealDamage(enemy, Game::starredValue({82, 125, 225}, star));
    }
}
