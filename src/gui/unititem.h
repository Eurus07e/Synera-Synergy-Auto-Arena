#ifndef GUI_ITEMS_UNITITEM_H
#define GUI_ITEMS_UNITITEM_H

#include <QGraphicsObject>
#include <QPoint>
#include <QPixmap>

class Unit;

class UnitItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit UnitItem(Unit* unit, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    [[nodiscard]] Unit* unit() const { return m_unit; }
    [[nodiscard]] int unitId() const;

    void setGridPos(const QPoint& gridPos);
    [[nodiscard]] QPoint gridPos() const { return m_gridPos; }

signals:
    void dragStarted(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragMoved(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void dragDropped(int unitId, const QPoint& sourceGrid, const QPointF& scenePos);
    void detailRequested(int unitId);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void ensureSpriteLoaded() const;
    [[nodiscard]] QString spriteRelativePathForUnit() const;

    Unit* m_unit;
    QPoint m_gridPos;
    bool m_dragging;
    mutable QPixmap m_sprite;
    mutable bool m_spriteTried;
};

#endif // GUI_ITEMS_UNITITEM_H
