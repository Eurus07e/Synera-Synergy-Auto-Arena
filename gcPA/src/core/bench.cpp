//
// Created by 束宇轩 on 2026/5/20.
//

#include "bench.h"
#include <algorithm>

Bench::Bench()
    : m_slots(SLOTS, nullptr)
{}

bool Bench::addUnit(Unit* unit)
{
    if (!unit) {
        return false;
    }

    for (auto & m_slot : m_slots) {
        if (!m_slot) {
            m_slot = unit;
            return true;
        }
    }

    return false;
}

bool Bench::placeUnit(Unit* unit, int slot)
{
    if (!unit || !isValidSlot(slot) || m_slots[slot]) {
        return false;
    }

    m_slots[slot] = unit;
    return true;
}

bool Bench::swapUnits(int firstSlot, int secondSlot)
{
    if (!isValidSlot(firstSlot) || !isValidSlot(secondSlot) || firstSlot == secondSlot) {
        return false;
    }

    std::swap(m_slots[firstSlot], m_slots[secondSlot]);
    return true;
}

void Bench::removeUnit(const Unit* unit)
{
    const int slot = findUnit(unit);
    if (slot >= 0) {
        m_slots[slot] = nullptr;
    }
}

Unit* Bench::getUnitAt(int slot) const
{
    if (!isValidSlot(slot)) {
        return nullptr;
    }

    return m_slots[slot];
}

bool Bench::hasUnitAt(int slot) const
{
    return getUnitAt(slot) != nullptr;
}

bool Bench::isValidSlot(int slot)
{
    return slot >= 0 && slot < SLOTS;
}

int Bench::findUnit(const Unit* unit) const
{
    if (!unit) {
        return -1;
    }

    for (int i = 0; i < m_slots.size(); ++i) {
        if (m_slots[i] == unit) {
            return i;
        }
    }

    return -1;
}

void Bench::clear()
{
    std::fill(m_slots.begin(), m_slots.end(), nullptr);
}
