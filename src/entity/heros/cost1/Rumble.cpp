//
// Created by 束宇轩 on 2026/5/20.
//

#include "Rumble.h"
#include "core/game.h"

Rumble::Rumble()
    : Unit("Rumble")
{
    m_star = 1;
    m_cost = 1;
    m_maxHp = 700;
    m_hp = 700;
    m_atk = 45;
    m_range = 1;
    m_maxMana = 110;
    m_mana = 40;
    m_armor = 40;
    m_magicResist = 40;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Yordle };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

// 技能：机械重组。兰博获得4秒护盾，并对目标附近区域内敌人造成魔法伤害。
void Rumble::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> area = game->skillAreaTargets(target, enemyOwner, 0);
    casterState.shield += Game::starredValue({350, 430, 550}, star);
    casterState.shieldSeconds = 4.0;
    for (Unit* enemy : area) {
        game->dealDamage(enemy, Game::starredValue({80, 120, 180}, star));
    }
}
