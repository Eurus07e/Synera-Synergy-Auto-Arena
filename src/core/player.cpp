//
// Created by 束宇轩 on 2026/5/20.
//

#include "player.h"
#include <algorithm>

Player::Player()
    : m_hp(kInitialHp)
    , m_gold(kInitialGold)
    , m_level(kInitialLevel)
    , m_round(kInitialRound)
    , m_levelProgress(0)
    , m_lossStreak(0)
{}

void Player::reset()
{
    m_hp = kInitialHp;
    m_gold = kInitialGold;
    m_level = kInitialLevel;
    m_round = kInitialRound;
    m_levelProgress = 0;
    m_lossStreak = 0;
}

void Player::restoreState(int hp, int gold, int level, int round, int levelProgress, int lossStreak)
{
    m_hp = std::clamp(hp, 0, kInitialHp);
    m_gold = std::max(gold, 0);
    m_level = std::clamp(level, kInitialLevel, kMaxLevel);
    m_round = std::max(round, kInitialRound);
    m_levelProgress = m_level == kMaxLevel ? 0 : std::max(levelProgress, 0);
    m_lossStreak = std::max(lossStreak, 0);
}

bool Player::spendGold(int cost)
{
    if ( cost <= 0 )return false;
    if ( canAfford(cost) ) {
        m_gold = m_gold - cost;
        return true;
    }

    return false;
}

void Player::setGold(int gold)
{
    m_gold = std::max(gold, 0);
}

void Player::takeDamage(int amount)
{
    if ( amount <= 0 )return;
    m_hp = std::max(m_hp - amount, 0);
}

bool Player::buyLevelProgress()
{
    if ( m_level == kMaxLevel || !spendGold(kLevelProgressPerPurchase) ) {
        return false;
    }

    m_levelProgress += kLevelProgressPerPurchase;
    tryApplyLevelUp();

    return true;
}

void Player::addLevelProgress(int amount)
{
    if (amount <= 0 || m_level == kMaxLevel) {
        return;
    }

    m_levelProgress += amount;
    tryApplyLevelUp();
}

bool Player::tryApplyLevelUp()
{
    bool leveledUp = false;
    if ( m_level == kMaxLevel )return leveledUp;
    while ( m_level < kMaxLevel )
    {
        int progressNeeded = kLevelUpCosts[m_level - 1];
        if ( m_levelProgress >= progressNeeded ) {
            m_levelProgress = m_levelProgress - progressNeeded;
            ++m_level;
            if ( m_level == kMaxLevel ){ m_levelProgress = 0; }
            leveledUp = true;
        }else break;
    }
    return leveledUp;
}

int Player::recordLoss()
{
    ++m_lossStreak;
    return m_lossStreak;
}
