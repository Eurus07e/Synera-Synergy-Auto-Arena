//
// Created by 束宇轩 on 2026/5/20.
//

#include "Yasuo.h"
#include "core/game.h"

Yasuo::Yasuo()
    : Unit("Yasuo")
{
    m_star = 1;
    m_cost = 2;
    m_maxHp = 750;
    m_hp = 750;
    m_atk = 45;
    m_range = 1;
    m_maxMana = 30;
    m_mana = 0;
    m_armor = 45;
    m_magicResist = 45;
    m_attackSpeed = 0.80;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Ionia };
    m_roles = { Role::Executioner };
    m_state = UnitState::Idle;
}

void Yasuo::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> area = game->skillAreaTargets(target, enemyOwner, 0);
    if (area.size() == 1) {
        game->dealDamage(target, Game::starredValue({103, 157, 233}, star) * 2);
    } else {
        for (Unit* enemy : area) {
            game->dealDamage(enemy, Game::starredValue({103, 157, 233}, star));
        }
    }
}
