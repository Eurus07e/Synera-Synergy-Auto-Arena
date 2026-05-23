#include "gui/unititem.h"
#include "entity/unit/unit.h"
#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QFileInfo>

namespace {
QString equipmentBadge(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword: return "剑";
    case EquipmentType::RecurveBow: return "弓";
    case EquipmentType::NeedlesslyLargeRod: return "棒";
    case EquipmentType::TearOfTheGoddess: return "泪";
    case EquipmentType::ChainVest: return "甲";
    case EquipmentType::NegatronCloak: return "斗";
    case EquipmentType::GiantsBelt: return "带";
    case EquipmentType::SparringGloves: return "拳";
    case EquipmentType::GuinsoosRageblade: return "鬼";
    case EquipmentType::SpearOfShojin: return "矛";
    case EquipmentType::Thornmail: return "棘";
    case EquipmentType::WarmogsArmor: return "狂";
    case EquipmentType::IronSword: return "铁";
    case EquipmentType::HasteGloves: return "速";
    case EquipmentType::BlueCrystal: return "蓝";
    }
    return {};
}
}

UnitItem::UnitItem(Unit* unit, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_unit(unit)
    , m_gridPos(-1, -1)
    , m_dragging(false)
    , m_spriteTried(false)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
}

QRectF UnitItem::boundingRect() const
{
    return QRectF(-48, -64, 96, 106);
}

void UnitItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    ensureSpriteLoaded();
    const bool isEnemy = m_unit != nullptr && m_unit->owner() == Owner::EnemyCtrl;
    const QColor ownerColor = isEnemy ? QColor(225, 95, 80) : QColor(85, 155, 230);
    const QColor hpColor = isEnemy ? QColor(235, 85, 75) : QColor(70, 205, 105);

    if (!m_sprite.isNull()) {
        const QRectF targetRect(-40, -40, 80, 80);
        painter->drawPixmap(targetRect, m_sprite, m_sprite.rect());
        painter->setPen(QPen(ownerColor, 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QRectF(-31, -31, 62, 62));
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(20, 20, 20, 110));
        painter->drawEllipse(QRectF(-14, 8, 28, 10));

        QPolygonF badge;
        badge << QPointF(0, -15)
              << QPointF(13, -7)
              << QPointF(13, 7)
              << QPointF(0, 15)
              << QPointF(-13, 7)
              << QPointF(-13, -7);

        painter->setPen(QPen(QColor(18, 18, 18), 1.5));
        painter->setBrush(ownerColor);
        painter->drawPolygon(badge);

        if (m_unit) {
            painter->setPen(Qt::white);
            QFont font = painter->font();
            font.setPointSize(12);
            font.setBold(true);
            painter->setFont(font);
            painter->drawText(QRectF(-13, -13, 26, 26), Qt::AlignCenter, m_unit->name().left(1));
        }
    }

    if (m_unit) {
        const QRectF labelRect(-34, -62, 68, 16);
        const QRectF hpBack(-34, -44, 68, 7);
        const QRectF manaBack(-34, -35, 68, 6);
        qreal hpRatio = 0.0;
        if (m_unit->maxHp() > 0) {
            hpRatio = static_cast<qreal>(m_unit->hp()) / static_cast<qreal>(m_unit->maxHp());
            if (hpRatio < 0.0) {
                hpRatio = 0.0;
            } else if (hpRatio > 1.0) {
                hpRatio = 1.0;
            }
        }
        qreal manaRatio = 0.0;
        if (m_unit->maxMana() > 0) {
            manaRatio = static_cast<qreal>(m_unit->mana()) / static_cast<qreal>(m_unit->maxMana());
            if (manaRatio < 0.0) {
                manaRatio = 0.0;
            } else if (manaRatio > 1.0) {
                manaRatio = 1.0;
            }
        }

        painter->setPen(QColor(255, 226, 120));
        QFont labelFont = painter->font();
        labelFont.setPointSize(12);
        labelFont.setBold(true);
        painter->setFont(labelFont);
        painter->drawText(QRectF(labelRect.left(), labelRect.top(), 24, labelRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          QString::number(m_unit->star()) + "*");

        painter->setPen(Qt::white);
        QFont nameFont = painter->font();
        nameFont.setPointSize(8);
        nameFont.setBold(true);
        painter->setFont(nameFont);
        painter->drawText(QRectF(labelRect.left() + 28, labelRect.top(), labelRect.width() - 28, labelRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          m_unit->name());

        painter->setPen(QPen(QColor(20, 20, 20), 1));
        painter->setBrush(QColor(35, 35, 35, 210));
        painter->drawRoundedRect(hpBack, 2, 2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(hpColor);
        painter->drawRoundedRect(QRectF(hpBack.left(), hpBack.top(), hpBack.width() * hpRatio, hpBack.height()), 2, 2);

        painter->setPen(QPen(QColor(20, 20, 20), 1));
        painter->setBrush(QColor(35, 35, 35, 210));
        painter->drawRoundedRect(manaBack, 2, 2);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(75, 150, 235));
        painter->drawRoundedRect(QRectF(manaBack.left(), manaBack.top(), manaBack.width() * manaRatio, manaBack.height()), 2, 2);

        int equipmentIndex = 0;
        for (EquipmentType equipment : m_unit->equipment()) {
            const QRectF badge(-34 + equipmentIndex * 23, 23, 21, 14);
            painter->setPen(QPen(QColor(218, 185, 92), 1));
            painter->setBrush(QColor(56, 48, 32, 225));
            painter->drawRoundedRect(badge, 2, 2);
            QFont itemFont = painter->font();
            itemFont.setPointSize(6);
            itemFont.setBold(true);
            painter->setFont(itemFont);
            painter->setPen(QColor(245, 218, 138));
            painter->drawText(badge, Qt::AlignCenter, equipmentBadge(equipment));
            ++equipmentIndex;
        }
    }
}

void UnitItem::ensureSpriteLoaded() const
{
    if (m_spriteTried) {
        return;
    }

    m_spriteTried = true;
    const QString relativePath = spriteRelativePathForUnit();
    if (relativePath.isEmpty()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString roots[] = {
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath()
    };

    QPixmap pix;
    for (const QString& root : roots) {
        if (root.isEmpty()) {
            continue;
        }
        pix.load(root + "/" + relativePath);
        if (!pix.isNull()) {
            break;
        }
    }

    if (pix.isNull()) {
        return;
    }

    m_sprite = pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QString UnitItem::spriteRelativePathForUnit() const
{
    if (!m_unit) {
        return QString();
    }

    const QString name = m_unit->name();
    if (name == QString::fromUtf8("战术")) {
        return QStringLiteral("assets/craftpix-reaper-man-chibi-2d-game-sprites/Reaper_Man_1/PNG/PNG Sequences/Idle/0_Reaper_Man_Idle_000.png");
    }
    if (name == QString::fromUtf8("弓手")) {
        return QStringLiteral("assets/craftpix-satyr-tiny-style-2d-sprites/PNG/Satyr_01/PNG Sequences/Idle/Satyr_01_Idle_000.png");
    }
    if (name == QString::fromUtf8("法师")) {
        return QStringLiteral("assets/craftpix-wraith-tiny-style-2d-sprites/PNG/Wraith_02/PNG Sequences/Idle/Wraith_02_Idle_000.png");
    }

    return QString();
}

int UnitItem::unitId() const
{
    return m_unit ? m_unit->id() : -1;
}

void UnitItem::setGridPos(const QPoint& gridPos)
{
    m_gridPos = gridPos;
}

void UnitItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        emit detailRequested(unitId());
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    emit dragStarted(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    emit dragMoved(unitId(), m_gridPos, event->scenePos());
    event->accept();
}

void UnitItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        return;
    }

    m_dragging = false;
    emit dragDropped(unitId(), m_gridPos, event->scenePos());
    event->accept();
}
