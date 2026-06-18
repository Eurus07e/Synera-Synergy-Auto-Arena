//
// Created by 束宇轩 on 2026/5/20.
//

#include "Sejuani.h"
#include "core/game.h"

Sejuani::Sejuani()
    : Unit("Sejuani")
{
    m_star = 1;
    m_cost = 3;
    m_maxHp = 900;
    m_hp = 900;
    m_atk = 60;
    m_range = 1;
    m_maxMana = 80;
    m_mana = 40;
    m_armor = 50;
    m_magicResist = 50;
    m_attackSpeed = 0.60;
    m_critRate = 0.25;
    m_owner = Owner::PlayerCtrl;
    m_origins = { Origin::Freljord };
    m_roles = { Role::Guardian };
    m_state = UnitState::Idle;
}

// 技能：凛冬之怒。瑟庄妮获得4秒护盾，对目标附近敌人造成伤害和冰冷；若主目标已被冰冷，则额外眩晕。
void Sejuani::castSkill(Game* game, Unit* target)
{
    CombatUnitState& casterState = game->combatState(this);
    ++casterState.skillCastCount;
    setMana(0);
    const int star = this->star();
    const Owner enemyOwner = target->owner();
    const QList<Unit*> area = game->skillAreaTargets(target, enemyOwner, 0);
    casterState.shield += Game::starredValue({525, 575, 775}, star);
    casterState.shieldSeconds = 4.0;
    const bool previouslyChilled = game->combatState(target).chillSeconds > 0.0;
    for (Unit* enemy : area) {
        game->dealDamage(enemy, Game::starredValue({70, 105, 170}, star));
        game->combatState(enemy).chillSeconds = 4.0;
    }
    if (previouslyChilled) {
        game->combatState(target).stunSeconds = 1.0;
    }
}
