//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_BENCH_H
#define SYNERA_STARTER_BENCH_H


#include <QPointF>
#include <QVector>

class Unit;

class Bench
{
public:
    static constexpr int SLOTS = 8;

    Bench();

    bool addUnit(Unit* unit);
    bool placeUnit(Unit* unit, int slot);
    bool swapUnits(int firstSlot, int secondSlot);
    void removeUnit(const Unit* unit);
    [[nodiscard]] Unit* getUnitAt(int slot) const;
    [[nodiscard]] bool hasUnitAt(int slot) const;
    static bool isValidSlot(int slot) ;
    int findUnit( const Unit* unit) const;
    void clear();

private:
    QVector<Unit*> m_slots;
};



#endif //SYNERA_STARTER_BENCH_H
