#include "gui/equipmentitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <utility>

EquipmentItem::EquipmentItem(int itemId, QString title, QString stats, QColor color, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_itemId(itemId)
    , m_title(std::move(title))
    , m_stats(std::move(stats))
    , m_color(color)
    , m_dragging(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF EquipmentItem::boundingRect() const
{
    return {-38.0, -25.0, 76.0, 50.0};
}

void EquipmentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(m_color.lighter(145), 2));
    painter->setBrush(QColor(37, 39, 47));
    painter->drawRoundedRect(boundingRect(), 5, 5);

    QFont titleFont = painter->font();
    titleFont.setPointSize(9);
    titleFont.setBold(true);
    painter->setFont(titleFont);
    painter->setPen(m_color.lighter(135));
    painter->drawText(QRectF(-34, -20, 68, 18), Qt::AlignCenter, m_title);

    QFont statFont = painter->font();
    statFont.setPointSize(8);
    statFont.setBold(false);
    painter->setFont(statFont);
    painter->setPen(QColor(225, 227, 235));
    painter->drawText(QRectF(-34, 1, 68, 18), Qt::AlignCenter, m_stats);
}

void EquipmentItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }
    m_dragging = true;
    emit dragStarted(m_itemId, event->scenePos());
    event->accept();
}

void EquipmentItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }
    emit dragMoved(m_itemId, event->scenePos());
    event->accept();
}

void EquipmentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        return;
    }
    m_dragging = false;
    emit dragDropped(m_itemId, event->scenePos());
    event->accept();
}
