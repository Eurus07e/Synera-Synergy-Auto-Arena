#include "gui/unititem.h"
#include "entity/unit/unit.h"
#include <QCoreApplication>
#include <QDir>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
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
    return {-34, -40, 68, 70};
}

void UnitItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    ensureSpriteLoaded();
    const bool isEnemy = m_unit != nullptr && m_unit->owner() == Owner::EnemyCtrl;
    const QColor ownerColor = isEnemy ? QColor(225, 95, 80) : QColor(85, 155, 230);
    const QColor hpColor = isEnemy ? QColor(235, 85, 75) : QColor(70, 205, 105);

    if (!m_sprite.isNull()) {
        constexpr QRectF avatarRect(-27, -20, 54, 48);
        constexpr qreal avatarRadius = 7.0;
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(18, 18, 22, 175));
        painter->drawRoundedRect(avatarRect.adjusted(-2, -2, 2, 2), avatarRadius + 2.0, avatarRadius + 2.0);

        QPainterPath clipPath;
        clipPath.addRoundedRect(avatarRect, avatarRadius, avatarRadius);
        painter->save();
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->setClipPath(clipPath);
        painter->drawPixmap(avatarRect, m_sprite, m_sprite.rect());
        painter->restore();

        painter->setPen(QPen(ownerColor, 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(avatarRect, avatarRadius, avatarRadius);
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
        constexpr QRectF labelRect(-27, -38, 54, 11);
        constexpr QRectF hpBack(-27, -27, 54, 5);
        constexpr QRectF manaBack(-27, -22, 54, 4);
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
        labelFont.setPointSize(8);
        labelFont.setBold(true);
        painter->setFont(labelFont);
        painter->drawText(QRectF(labelRect.left(), labelRect.top(), 15, labelRect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          QString::number(m_unit->star()) + "*");

        painter->setPen(Qt::white);
        QFont nameFont = painter->font();
        nameFont.setPointSize(6);
        nameFont.setBold(true);
        painter->setFont(nameFont);
        painter->drawText(QRectF(labelRect.left() + 17, labelRect.top(), labelRect.width() - 17, labelRect.height()),
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
            const QRectF badge(-27 + equipmentIndex * 18, 17, 16, 10);
            painter->setPen(QPen(QColor(218, 185, 92), 1));
            painter->setBrush(QColor(56, 48, 32, 225));
            painter->drawRoundedRect(badge, 2, 2);
            QFont itemFont = painter->font();
            itemFont.setPointSize(5);
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
    const QString currentDir = QDir::currentPath();
    const QString roots[] = {
        currentDir,
        QFileInfo(currentDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/..").canonicalFilePath(),
        QFileInfo(appDir + "/../..").canonicalFilePath(),
        QFileInfo(appDir + "/../../..").canonicalFilePath(),
        QFileInfo(appDir + "/../../../..").canonicalFilePath()
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

    m_sprite = pix;
}

QString UnitItem::spriteRelativePathForUnit() const
{
    if (!m_unit) {
        return {};
    }

    const QString name = m_unit->name();
    if (name == QStringLiteral("Jarvan IV")) {
        return QStringLiteral("assets/hero_portraits/jarvan_iv.png");
    }
    if (name == QStringLiteral("Jhin")) {
        return QStringLiteral("assets/hero_portraits/jhin.png");
    }
    if (name == QStringLiteral("Rumble")) {
        return QStringLiteral("assets/hero_portraits/rumble.png");
    }
    if (name == QStringLiteral("Sona")) {
        return QStringLiteral("assets/hero_portraits/sona.png");
    }
    if (name == QStringLiteral("Ashe")) {
        return QStringLiteral("assets/hero_portraits/ashe.png");
    }
    if (name == QStringLiteral("Cho'Gath")) {
        return QStringLiteral("assets/hero_portraits/chogath.png");
    }
    if (name == QStringLiteral("Xin Zhao")) {
        return QStringLiteral("assets/hero_portraits/xin_zhao.png");
    }
    if (name == QStringLiteral("Yasuo")) {
        return QStringLiteral("assets/hero_portraits/yasuo.png");
    }
    if (name == QStringLiteral("Ahri")) {
        return QStringLiteral("assets/hero_portraits/ahri.png");
    }
    if (name == QStringLiteral("Jinx")) {
        return QStringLiteral("assets/hero_portraits/jinx.png");
    }
    if (name == QStringLiteral("Loris")) {
        return QStringLiteral("assets/hero_portraits/loris.png");
    }
    if (name == QStringLiteral("Sejuani")) {
        return QStringLiteral("assets/hero_portraits/sejuani.png");
    }

    return {};
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
