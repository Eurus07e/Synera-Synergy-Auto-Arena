//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_PLAYER_H
#define SYNERA_STARTER_PLAYER_H


class Player
{
public:
    Player();

    void reset();
    void restoreState(int hp, int gold, int level, int round, int levelProgress, int lossStreak);

    [[nodiscard]]int hp() const { return m_hp; }
    [[nodiscard]]int gold() const { return m_gold; }
    [[nodiscard]]int level() const { return m_level; }
    [[nodiscard]]int round() const { return m_round; }
    [[nodiscard]]int unitCap() const { return m_level; }
    [[nodiscard]]int levelProgress() const { return m_levelProgress; }
    [[nodiscard]]int lossStreak() const { return m_lossStreak; }
    [[nodiscard]]int levelProgressNeeded() const
    {
        return m_level >= kMaxLevel ? 0 : kLevelUpCosts[m_level - 1];
    }

    [[nodiscard]]bool canAfford(int cost) const { return m_gold >= cost && cost>0 ; }
    bool spendGold(int cost);
    void setGold(int gold);
    void addGold(int amount) { if ( amount>0 ) m_gold = m_gold + amount; }

    void takeDamage(int amount);
    [[nodiscard]]bool isDead() const { return m_hp == 0; }

    bool buyLevelProgress();
    void addLevelProgress(int amount);
    bool tryApplyLevelUp();
    void advanceRound() { ++m_round; }
    void recordWin() { m_lossStreak = 0; }
    int recordLoss();

private:
    static constexpr int kInitialHp = 100;
    static constexpr int kInitialGold = 6;
    static constexpr int kInitialLevel = 3;
    static constexpr int kInitialRound = 1;
    static constexpr int kMaxLevel = 10;
    static constexpr int kLevelProgressPerPurchase = 4;
    static constexpr int kLevelUpCosts[kMaxLevel - 1] = { 4, 4, 6, 10, 15, 30, 36, 60, 70 };

    int m_hp;
    int m_gold;
    int m_level;
    int m_round;
    int m_levelProgress;
    int m_lossStreak;
};


#endif //SYNERA_STARTER_PLAYER_H
