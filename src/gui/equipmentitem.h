#ifndef GUI_EQUIPMENTITEM_H
#define GUI_EQUIPMENTITEM_H

#include <QColor>
#include <QGraphicsObject>
#include <QString>

class EquipmentItem : public QGraphicsObject
{
    Q_OBJECT

public:
    EquipmentItem(int itemId, QString title, QString stats, QColor color, QGraphicsItem* parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    [[nodiscard]] int itemId() const { return m_itemId; }

signals:
    void dragStarted(int itemId, const QPointF& scenePos);
    void dragMoved(int itemId, const QPointF& scenePos);
    void dragDropped(int itemId, const QPointF& scenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int m_itemId;
    QString m_title;
    QString m_stats;
    QColor m_color;
    bool m_dragging;
};

#endif
