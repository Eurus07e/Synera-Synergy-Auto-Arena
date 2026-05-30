#ifndef BOARD_H
#define BOARD_H

#include <QHash>
#include <QPoint>
#include <QVector>
#include "entity/unit/unit.h"

class Board
{
public:
    static constexpr int ROWS = 8;
    static constexpr int COLS = 8;

    Board();
    ~Board() = default;

    void addUnit(Unit* unit, const QPoint& pos);
    void removeUnit(Unit* unit);
    Unit* getUnitAt(const QPoint& pos) const;
    bool hasUnitAt(const QPoint& pos) const;

    [[nodiscard]] static bool isValidPosition(const QPoint& pos);
    [[nodiscard]] static bool isPlayerHalf(const QPoint& pos);

    void clear();

private:
    [[nodiscard]] static int indexOf(const QPoint& pos);

    QVector<Unit*> m_cells;
    QHash<Unit*, QPoint> m_unitToPosition;
};

#endif // BOARD_H
