#include "gamewindow.h"
#include "core/game.h"
#include "core/player.h"
#include "entity/unit/unit.h"
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QEasingCurve>
#include <QFileInfo>
#include <QFrame>
#include <QFontMetrics>
#include <QGridLayout>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <cmath>
#include <memory>

namespace {
constexpr int kShopPanelExpandedHeight = 276;
constexpr int kShopPanelAnimationMs = 180;

class ZoomableGraphicsView : public QGraphicsView
{
public:
    explicit ZoomableGraphicsView(QWidget* parent = nullptr)
        : QGraphicsView(parent)
    {
    }

protected:
    bool event(QEvent* event) override
    {
        return handleNativeGesture(event) || QGraphicsView::event(event);
    }

    bool viewportEvent(QEvent* event) override
    {
        return handleNativeGesture(event) || QGraphicsView::viewportEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        if (event->modifiers().testFlag(Qt::ControlModifier)
            || event->modifiers().testFlag(Qt::MetaModifier)) {
            applyWheelZoom(event);
            return;
        }

        panByWheel(event);
    }

private:
    bool handleNativeGesture(QEvent* event)
    {
        if (event->type() != QEvent::NativeGesture) {
            return false;
        }

        auto* gesture = static_cast<QNativeGestureEvent*>(event);
        if (gesture->gestureType() != Qt::ZoomNativeGesture) {
            return false;
        }

        applyZoom(std::exp(gesture->value()), gesture);
        return true;
    }

    void applyZoom(qreal requestedFactor, QInputEvent* event)
    {
        constexpr qreal minScale = 0.55;
        constexpr qreal maxScale = 2.30;

        const qreal currentScale = transform().m11();
        if (currentScale <= 0.0 || requestedFactor <= 0.0) {
            event->ignore();
            return;
        }

        const qreal wantedScale = qBound(minScale, currentScale * requestedFactor, maxScale);
        const qreal factor = wantedScale / currentScale;
        if (qFuzzyCompare(factor, 1.0)) {
            event->accept();
            return;
        }

        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        scale(factor, factor);
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        event->accept();
    }

    void applyWheelZoom(QWheelEvent* event)
    {
        constexpr qreal zoomBase = 1.15;
        const qreal delta = !event->angleDelta().isNull()
            ? static_cast<qreal>(event->angleDelta().y()) / 120.0
            : static_cast<qreal>(event->pixelDelta().y()) / 240.0;
        if (qFuzzyIsNull(delta)) {
            event->ignore();
            return;
        }

        applyZoom(std::pow(zoomBase, delta), event);
    }

    void panByWheel(QWheelEvent* event)
    {
        QPointF delta = event->pixelDelta();
        if (delta.isNull()) {
            constexpr qreal wheelStepPixels = 48.0;
            delta = QPointF(event->angleDelta()) / 120.0 * wheelStepPixels;
        }

        if (event->modifiers().testFlag(Qt::ShiftModifier) && qFuzzyIsNull(delta.x())) {
            delta.setX(delta.y());
            delta.setY(0.0);
        }

        if (delta.isNull()) {
            event->ignore();
            return;
        }

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - qRound(delta.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - qRound(delta.y()));
        event->accept();
    }
};

template <typename T>
void releaseToQt(std::unique_ptr<T>& object)
{
    [[maybe_unused]] T* qtOwnedObject = object.release();
}

QString singleSaveSlotPath()
{
    const QString saveDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    [[maybe_unused]] const bool directoryReady = QDir().mkpath(saveDirectory);
    return QDir(saveDirectory).filePath("synera_save.json");
}

QString originToText(Origin origin)
{
    switch (origin) {
    case Origin::Demacia: return "Demacia 德玛西亚";
    case Origin::Noxus: return "Noxus 诺克萨斯";
    case Origin::Ionia: return "Ionia 艾欧尼亚";
    case Origin::Freljord: return "Freljord 弗雷尔卓德";
    case Origin::Bilgewater: return "Bilgewater 比尔吉沃特";
    case Origin::Zaun: return "Zaun 祖安";
    case Origin::Piltover: return "Piltover 皮尔特沃夫";
    case Origin::ShadowIsles: return "Shadow Isles 暗影岛";
    case Origin::Targon: return "Targon 巨神峰";
    case Origin::Shurima: return "Shurima 恕瑞玛";
    case Origin::Yordle: return "Yordle 约德尔人";
    case Origin::Void: return "Void 虚空";
    }
    return "Unknown";
}

QString roleToText(Role role)
{
    switch (role) {
    case Role::Warrior: return "Warrior 战士";
    case Role::Mage: return "Mage 法师";
    case Role::Assassin: return "Assassin 刺客";
    case Role::Ranger: return "Ranger 射手";
    case Role::Vanguard: return "Vanguard 重装";
    case Role::Mystic: return "Mystic 秘术";
    case Role::Brawler: return "Brawler 斗士";
    case Role::Protector: return "Protector 神盾使";
    case Role::Swiftshot: return "Swiftshot 迅捷";
    case Role::Guardian: return "Guardian 护卫";
    case Role::Executioner: return "Executioner 裁决";
    case Role::Scholar: return "Scholar 学者";
    case Role::Oracle: return "Oracle 神谕者";
    case Role::Gunner: return "Gunner 枪手";
    case Role::Sniper: return "Sniper 狙神";
    case Role::Dominator: return "Dominator 主宰";
    }
    return "Unknown";
}

QString originsText(const ShopSlot& slot)
{
    QStringList origins;
    for (const Origin& origin : slot.origins) {
        origins << originToText(origin);
    }

    return origins.join(", ");
}

QString rolesText(const ShopSlot& slot)
{
    QStringList roles;
    for (const Role& role : slot.roles) {
        roles << roleToText(role);
    }

    return roles.join(", ");
}

QString originsText(const Unit* unit)
{
    QStringList origins;
    for (const Origin& origin : unit->origins()) {
        origins << originToText(origin);
    }

    return origins.join(", ");
}

QString rolesText(const Unit* unit)
{
    QStringList roles;
    for (const Role& role : unit->roles()) {
        roles << roleToText(role);
    }

    return roles.join(", ");
}

QString positionTypeText(UnitPositionType positionType)
{
    return positionType == UnitPositionType::Frontline
        ? "前排"
        : "后排";
}

QString equipmentName(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword: return "暴风大剑";
    case EquipmentType::RecurveBow: return "反曲之弓";
    case EquipmentType::NeedlesslyLargeRod: return "无用大棒";
    case EquipmentType::TearOfTheGoddess: return "女神之泪";
    case EquipmentType::ChainVest: return "锁子甲";
    case EquipmentType::NegatronCloak: return "负极斗篷";
    case EquipmentType::GiantsBelt: return "巨人腰带";
    case EquipmentType::SparringGloves: return "拳套";
    case EquipmentType::GuinsoosRageblade: return "鬼索的狂暴之刃";
    case EquipmentType::SpearOfShojin: return "朔极之矛";
    case EquipmentType::Thornmail: return "荆棘背心";
    case EquipmentType::WarmogsArmor: return "狂徒铠甲";
    case EquipmentType::IronSword: return "铁剑";
    case EquipmentType::HasteGloves: return "急速手套";
    case EquipmentType::BlueCrystal: return "蓝水晶";
    }
    return {};
}

QString equipmentDescription(EquipmentType type)
{
    switch (type) {
    case EquipmentType::BFSword:
        return "基础散件\n攻击力 +10";
    case EquipmentType::RecurveBow:
        return "基础散件\n攻击速度 +10%";
    case EquipmentType::NeedlesslyLargeRod:
        return "基础散件\n法术强度 +10";
    case EquipmentType::TearOfTheGoddess:
        return "基础散件\n初始法力值 +15";
    case EquipmentType::ChainVest:
        return "基础散件\n生命值 +150";
    case EquipmentType::NegatronCloak:
        return "基础散件\n魔法抗性 +20";
    case EquipmentType::GiantsBelt:
        return "基础散件\n生命值 +150";
    case EquipmentType::SparringGloves:
        return "基础散件\n暴击几率 +20%";
    case EquipmentType::GuinsoosRageblade:
        return "合成：反曲之弓 + 无用大棒\n"
               "攻击速度 +10%，法术强度 +10\n\n"
               "被动：每次普通攻击后，攻击速度增加 6%，无限叠加。";
    case EquipmentType::SpearOfShojin:
        return "合成：暴风大剑 + 女神之泪\n"
               "攻击力 +10，初始法力值 +15\n\n"
               "被动：普通攻击命中后回复 5 点法力值。";
    case EquipmentType::Thornmail:
        return "合成：锁子甲 + 锁子甲\n"
               "生命值 +300\n\n"
               "被动：受到普通攻击时反弹 25 点伤害；"
               "受到的暴击普通攻击伤害减少 10%。";
    case EquipmentType::WarmogsArmor:
        return "合成：巨人腰带 + 巨人腰带\n"
               "生命值 +300\n\n"
               "被动：3 秒未受到攻击后，每秒回复 5% 最大生命值；"
               "被攻击后重新计时。";
    case EquipmentType::IronSword:
        return "助教要求基础装备\n攻击力 +15";
    case EquipmentType::HasteGloves:
        return "助教要求基础装备\n攻击速度提升 20%";
    case EquipmentType::BlueCrystal:
        return "助教要求基础装备\n最大法力值 -30\n\n"
               "作用：降低施法所需法力阈值，使技能更快释放。";
    }
    return {};
}

QString equipmentText(const Unit& unit)
{
    if (unit.equipment().empty()) {
        return "装备：无";
    }
    QStringList items;
    for (const EquipmentType& equipment : unit.equipment()) {
        items << equipmentName(equipment);
    }
    return "装备：" + items.join(" | ");
}

QString heroLocalizedName(HeroType heroType)
{
    switch (heroType) {
    case HeroType::JarvanIV: return "嘉文四世";
    case HeroType::Jhin: return "烬";
    case HeroType::Rumble: return "兰博";
    case HeroType::Sona: return "娑娜";
    case HeroType::Ashe: return "艾希";
    case HeroType::ChoGath: return "科加斯";
    case HeroType::XinZhao: return "赵信";
    case HeroType::Yasuo: return "亚索";
    case HeroType::Ahri: return "阿狸";
    case HeroType::Jinx: return "金克丝";
    case HeroType::Loris: return "洛里斯";
    case HeroType::Sejuani: return "瑟庄妮";
    }
    return {};
}

QString heroLocalizedName(const QString& name)
{
    if (name == "Jarvan IV") return "嘉文四世";
    if (name == "Jhin") return "烬";
    if (name == "Rumble") return "兰博";
    if (name == "Sona") return "娑娜";
    if (name == "Ashe") return "艾希";
    if (name == "Cho'Gath") return "科加斯";
    if (name == "Xin Zhao") return "赵信";
    if (name == "Yasuo") return "亚索";
    if (name == "Ahri") return "阿狸";
    if (name == "Jinx") return "金克丝";
    if (name == "Loris") return "洛里斯";
    if (name == "Sejuani") return "瑟庄妮";
    return {};
}

QString shopStatsText(const ShopSlot& slot)
{

    return QString("<table width='100%' cellspacing='0' cellpadding='1'>"
                   "<tr>"
                   "<td width='31%'>Health</td><td width='18%' align='right'>%1</td>"
                   "<td width='2%' align='center'><font color='#5f6678'>|</font></td>"
                   "<td width='31%'>Attack Damage</td><td width='18%' align='right'>%2</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Attack Range</td><td align='right'>%3</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Mana</td><td align='right'>%4 / %5</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Armor</td><td align='right'>%6</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Magic Resist</td><td align='right'>%7</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Attack Speed</td><td align='right'>%8</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Critical Rate</td><td align='right'>%9%</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Position</td><td align='right'>%10</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td></td><td></td>"
                   "</tr>"
                   "</table>")
        .arg(slot.maxHp)
        .arg(slot.atk)
        .arg(slot.range)
        .arg(slot.mana)
        .arg(slot.maxMana)
        .arg(slot.armor)
        .arg(slot.magicResist)
        .arg(slot.attackSpeed, 0, 'f', 2)
        .arg(slot.critRate * 100.0, 0, 'f', 0)
        .arg(positionTypeText(slot.positionType));
}

QString unitStatsText(const Unit* unit)
{
    // 右键详情卡用双列参数表，减少横向留白，让数值区在较大字号下仍然紧凑。
    return QString("<table width='100%' cellspacing='0' cellpadding='3'>"
                   "<tr>"
                   "<td width='23%'>Cost</td><td width='25%' align='right'>%1 gold</td>"
                   "<td width='4%' align='center'><font color='#5f6678'>|</font></td>"
                   "<td width='23%'>Star</td><td width='25%' align='right'>%2</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Health</td><td align='right'>%3 / %4</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Attack Damage</td><td align='right'>%5</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Attack Range</td><td align='right'>%6</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Mana</td><td align='right'>%7 / %8</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Armor</td><td align='right'>%9</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Magic Resist</td><td align='right'>%10</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Attack Speed</td><td align='right'>%11</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Critical Rate</td><td align='right'>%12%</td>"
                   "</tr>"
                   "<tr>"
                   "<td>Position</td><td align='right'>%13</td>"
                   "<td align='center'><font color='#5f6678'>|</font></td>"
                   "<td>Ability Power</td><td align='right'>%14</td>"
                   "</tr>"
                   "</table>")
        .arg(unit->cost())
        .arg(unit->star())
        .arg(unit->hp())
        .arg(unit->maxHp())
        .arg(unit->atk())
        .arg(unit->range())
        .arg(unit->mana())
        .arg(unit->maxMana())
        .arg(unit->armor())
        .arg(unit->magicResist())
        .arg(unit->attackSpeed(), 0, 'f', 2)
        .arg(unit->critRate() * 100.0, 0, 'f', 0)
        .arg(positionTypeText(unit->positionType()))
        .arg(unit->abilityPower());
}

QString skillDescriptionForUnitName(const QString& name)
{
    if (name == "Jarvan IV") return "德邦军旗：获得持续4秒的护盾，并给全场友军增加持续4秒的攻速。\nShield: 350 / 425 / 500\nAttack Speed Bonus: 20% / 25% / 50%";
    if (name == "Jhin") return "完美谢幕：接下来4次普攻攻速设为1并获得无限射程，普攻替换为加农炮击；第4炮额外造成144%伤害。\nBase Damage: 155 / 235 / 350\nAbility Scaling: 15 / 22 / 34";
    if (name == "Rumble") return "机械重组：获得持续4秒的护盾，并发射火焰，在一个锥形内造成魔法伤害。\nShield: 350 / 430 / 550\nDamage: 180% / 270% / 405% Armor";
    if (name == "Sona") return "能量和弦：对2个附近敌人造成魔法伤害，同时为生命值最低的友军回复生命值。\nDamage: 130 / 195 / 300\nHealing: 40 / 50 / 80";
    if (name == "Ashe") return "臻冰之箭：对目标造成物理伤害，并对半径1格圆形区域内的邻格敌人造成伤害；低于30%最大生命值的目标改为受到真实伤害。命中的敌人被施加持续3秒的30%冰冷效果。\nDamage: 135 / 195 / 300 AD + 20 / 30 / 40 AP\nArea Damage: 33% skill damage";
    if (name == "Cho'Gath") return "碎裂：获得10永久最大生命值，回复生命值，并碎裂目标脚下半径2格的范围。短暂延迟后，范围内敌人被击飞1.5秒并受到魔法伤害。\nHealing: 200 / 225 / 400\nDamage: 5% max health + 45 / 75 / 110 AP";
    if (name == "Xin Zhao") return "三重爪击：打击目标3次。每次打击造成物理伤害并回复生命值，最后一击造成1.5秒晕眩。\nDamage: 65 / 100 / 150\nHealing: 105 / 145 / 200";
    if (name == "Yasuo") return "踏前斩：冲刺，随后对邻格敌人造成物理伤害。如果仅有一个敌人被命中，则造成双倍伤害。\nDamage: 95 / 145 / 215 AD + 8 / 12 / 18 AP\nBonus: single-target hit deals double damage";
    if (name == "Ahri") return "狐火：向目标投出3团狐火，每团狐火造成魔法伤害。每第3次施放，投出9团狐火，在目标和至多2名附近敌人之间分摊；如果附近有敌人，则冲刺远离。\nDamage: 82 / 125 / 225 AP per foxfire";
    if (name == "Jinx") return "枪炮交响曲：被动。攻击指定次数后切换至鱼骨头；鱼骨头状态下，攻击替换为3枚导弹，导弹瞄准随机敌人并造成物理伤害。\nAttack Count: 18 / 18 / 16\nDamage: 54 / 82 / 150 AD + 4 / 6 / 9 AP";
    if (name == "Loris") return "皮城争斗：获得持续4秒的护盾，冲锋并击退目标，对目标和命中的单位造成1.25秒晕眩和魔法伤害。冲锋后，迫使附近敌人瞄准洛里斯。\nShield: 700 / 800 / 1000\nDamage: 150 / 225 / 360";
    if (name == "Sejuani") return "凛冬之怒：获得持续4秒的护盾，打击一个锥形和一条直线。被命中的敌人受到魔法伤害和持续4秒的30%冰冷；如果目标已被冰冷，则晕眩1秒。\nShield: 525 / 575 / 775\nDamage: 70 / 105 / 170";
    return {};
}

QString unitSkillText(const Unit* unit)
{
    const QString description = skillDescriptionForUnitName(unit->name());
    const qsizetype colonIndex = description.indexOf(u'：');
    if (colonIndex < 0) {
        return description.toHtmlEscaped().replace("\n", "<br>");
    }

    const QString title = description.left(colonIndex).toHtmlEscaped();
    const QString body = description.mid(colonIndex + 1).trimmed().toHtmlEscaped().replace("\n", "<br>");
    return QString("<div style='font-weight:800; color:#f0d37a; font-size:16px;'>%1</div>"
                   "<div style='margin-top:8px;'>%2</div>")
        .arg(title, body);
}

QString skillText(const ShopSlot& slot)
{
    const QString description = slot.skillDescription;
    // 技能描述里用中文冒号分隔“技能名”和“技能正文”，界面上把技能名单独高亮显示。
    const qsizetype colonIndex = description.indexOf(u'：');
    if (colonIndex < 0) {
        return description.toHtmlEscaped();
    }

    const QString title = description.left(colonIndex).toHtmlEscaped();
    const QString body = description.mid(colonIndex + 1).trimmed().toHtmlEscaped().replace("\n", "<br>");
    return QString("<div style='font-weight:800; color:#f0d37a;'>%1</div>"
                   "<div style='margin-top:4px;'>%2</div>")
        .arg(title, body);
}

QString statusButtonStyle(bool affordable)
{
    const QString background = affordable ? "#245c36" : "#202126";
    const QString border = affordable ? "#57c878" : "#4a4d58";
    const QString hover = affordable ? "#2f7444" : "#2a2d35";
    const QString pressed = affordable ? "#1d472b" : "#181a1f";
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: #f2f2f2;
            border: 1px solid %2;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: %3;
            border-color: %2;
        }
        QPushButton:pressed {
            background-color: %4;
        }
        QPushButton:disabled {
            background-color: #272a31;
            color: #8d93a0;
            border-color: #454956;
        }
    )").arg(background, border, hover, pressed);
}

QString heroPortraitRelativePath(HeroType heroType)
{
    switch (heroType) {
    case HeroType::JarvanIV: return QStringLiteral("assets/hero_portraits/jarvan_iv.png");
    case HeroType::Jhin: return QStringLiteral("assets/hero_portraits/jhin.png");
    case HeroType::Rumble: return QStringLiteral("assets/hero_portraits/rumble.png");
    case HeroType::Sona: return QStringLiteral("assets/hero_portraits/sona.png");
    case HeroType::Ashe: return QStringLiteral("assets/hero_portraits/ashe.png");
    case HeroType::ChoGath: return QStringLiteral("assets/hero_portraits/chogath.png");
    case HeroType::XinZhao: return QStringLiteral("assets/hero_portraits/xin_zhao.png");
    case HeroType::Yasuo: return QStringLiteral("assets/hero_portraits/yasuo.png");
    case HeroType::Ahri: return QStringLiteral("assets/hero_portraits/ahri.png");
    case HeroType::Jinx: return QStringLiteral("assets/hero_portraits/jinx.png");
    case HeroType::Loris: return QStringLiteral("assets/hero_portraits/loris.png");
    case HeroType::Sejuani: return QStringLiteral("assets/hero_portraits/sejuani.png");
    }
    return {};
}

QPixmap loadHeroPortrait(HeroType heroType)
{
    const QString relativePath = heroPortraitRelativePath(heroType);
    if (relativePath.isEmpty()) {
        return {};
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

    QPixmap pixmap;
    for (const QString& root : roots) {
        if (root.isEmpty()) {
            continue;
        }
        pixmap.load(root + "/" + relativePath);
        if (!pixmap.isNull()) {
            return pixmap;
        }
    }
    return {};
}

QString compactTraitName(const QString& text)
{
    const qsizetype split = text.indexOf(' ');
    if (split >= 0 && split + 1 < text.size()) {
        return text.mid(split + 1);
    }
    return text;
}

QStringList compactTraitLines(const ShopSlot& slot)
{
    QStringList traits;
    for (const Role& role : slot.roles) {
        traits << compactTraitName(roleToText(role));
    }
    for (const Origin& origin : slot.origins) {
        traits << compactTraitName(originToText(origin));
    }
    return traits;
}

class ShopCardButton final : public QPushButton
{
public:
    explicit ShopCardButton(QWidget* parent = nullptr)
        : QPushButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setMouseTracking(true);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setFixedSize(172, 244);
        setToolTip("左键单击卡牌立即购买；右键或触控板双指点按查看详情");
    }

    void setShopSlot(const ShopSlot* slot, bool canBuy, bool alreadyOwned)
    {
        if (slot == nullptr) {
            m_hasSlot = false;
            m_detailMode = false;
            m_portrait = {};
            setEnabled(false);
            setToolTip("该商店槽位已售出，刷新商店可获得新英雄");
            update();
            return;
        }

        const bool changedHero = !m_hasSlot
            || m_slot.heroType != slot->heroType
            || m_slot.heroName != slot->heroName
            || m_slot.cost != slot->cost;
        m_slot = *slot;
        m_hasSlot = true;
        m_canBuy = canBuy;
        m_alreadyOwned = alreadyOwned;
        setEnabled(true);
        if (changedHero) {
            m_detailMode = false;
            m_portrait = loadHeroPortrait(m_slot.heroType);
        }
        refreshToolTip();
        update();
    }

    [[nodiscard]] QSize sizeHint() const override
    {
        return {172, 244};
    }

    [[nodiscard]] QSize minimumSizeHint() const override
    {
        return {160, 228};
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (!m_hasSlot) {
            paintEmpty(&painter);
            return;
        }
        if (m_detailMode) {
            paintDetails(&painter);
        } else {
            paintSummary(&painter);
        }
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::RightButton && m_hasSlot) {
            m_ignoreNextContextMenu = true;
            toggleDetailMode();
            event->accept();
            return;
        }

        if (event->button() != Qt::LeftButton || !m_hasSlot) {
            QPushButton::mousePressEvent(event);
            return;
        }
        m_pressed = true;
        update();
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton || !m_hasSlot) {
            QPushButton::mouseReleaseEvent(event);
            return;
        }

        const bool wasPressed = m_pressed;
        m_pressed = false;
        update();
        if (!wasPressed || !rect().contains(event->pos())) {
            event->accept();
            return;
        }

        if (m_detailMode) {
            if (backButtonRect().contains(event->pos())) {
                exitDetailMode();
            }
            event->accept();
            return;
        }

        if (m_canBuy) {
            click();
        }
        event->accept();
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() != Qt::LeftButton || !m_hasSlot) {
            QPushButton::mouseDoubleClickEvent(event);
            return;
        }

        if (m_detailMode) {
            exitDetailMode();
        }
        event->accept();
    }

    void contextMenuEvent(QContextMenuEvent* event) override
    {
        if (!m_hasSlot) {
            QPushButton::contextMenuEvent(event);
            return;
        }

        if (m_ignoreNextContextMenu) {
            m_ignoreNextContextMenu = false;
            event->accept();
            return;
        }

        toggleDetailMode();
        event->accept();
    }

private:
    [[nodiscard]] QRect backButtonRect() const
    {
        return {width() - 54, 8, 44, 22};
    }

    [[nodiscard]] QRectF summaryBottomRect() const
    {
        const QRectF content = rect().adjusted(8, 8, -8, -8);
        constexpr qreal bottomHeight = 58.0;
        return {content.left(), content.bottom() - bottomHeight + 1.0, content.width(), bottomHeight};
    }

    [[nodiscard]] QRectF summaryImageRect() const
    {
        const QRectF content = rect().adjusted(8, 8, -8, -8);
        const QRectF bottomRect = summaryBottomRect();
        return {content.left(), content.top(), content.width(), bottomRect.top() - content.top() + 8.0};
    }

    void enterDetailMode()
    {
        m_pressed = false;
        m_detailMode = true;
        refreshToolTip();
        update();
    }

    void exitDetailMode()
    {
        m_pressed = false;
        m_detailMode = false;
        refreshToolTip();
        update();
    }

    void toggleDetailMode()
    {
        if (m_detailMode) {
            exitDetailMode();
        } else {
            enterDetailMode();
        }
    }

    void refreshToolTip()
    {
        if (!m_hasSlot) {
            setToolTip("该商店槽位已售出，刷新商店可获得新英雄");
            return;
        }

        if (m_detailMode) {
            setToolTip("左键点击返回按钮关闭详情；右键或触控板双指点按也可关闭");
            return;
        }

        setToolTip(m_canBuy
            ? "左键单击整张卡牌立即购买；右键或触控板双指点按查看详情"
            : "金币不足或备战席已满；右键或触控板双指点按仍可查看详情");
    }

    void paintFrame(QPainter* painter, const QColor& borderColor)
    {
        const QRectF cardRect = rect().adjusted(2, 2, -2, -2);
        painter->setPen(QPen(borderColor, m_alreadyOwned ? 2.5 : 1.6));
        painter->setBrush(QColor(35, 38, 48));
        painter->drawRoundedRect(cardRect, 8, 8);
    }

    void paintEmpty(QPainter* painter)
    {
        paintFrame(painter, QColor(76, 82, 97));
        painter->setPen(QColor(150, 156, 172));
        QFont font = painter->font();
        font.setPointSize(13);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap,
                          "Sold\nRefresh the shop to roll a new hero.");
    }

    void paintSummary(QPainter* painter)
    {
        const QColor borderColor = m_alreadyOwned ? QColor(88, 209, 122) : QColor(210, 170, 64);
        paintFrame(painter, borderColor);

        const QRectF imageRect = summaryImageRect();
        const QRectF bottomRect = summaryBottomRect();

        QPainterPath clip;
        clip.addRoundedRect(imageRect, 6, 6);
        painter->save();
        painter->setClipPath(clip);
        if (!m_portrait.isNull()) {
            const QSize targetSize = imageRect.size().toSize();
            const QPixmap scaled = m_portrait.scaled(targetSize, Qt::KeepAspectRatioByExpanding,
                                                     Qt::SmoothTransformation);
            const QPointF topLeft(imageRect.center().x() - scaled.width() * 0.5,
                                  imageRect.center().y() - scaled.height() * 0.5);
            painter->drawPixmap(topLeft, scaled);
        } else {
            QLinearGradient gradient(imageRect.topLeft(), imageRect.bottomRight());
            gradient.setColorAt(0.0, QColor(62, 68, 92));
            gradient.setColorAt(1.0, QColor(28, 31, 42));
            painter->fillRect(imageRect, gradient);
        }
        painter->restore();

        QLinearGradient imageShade(imageRect.bottomLeft(), imageRect.topLeft());
        imageShade.setColorAt(0.0, QColor(0, 0, 0, 180));
        imageShade.setColorAt(0.55, QColor(0, 0, 0, 20));
        painter->fillRect(imageRect, imageShade);

        const QStringList traitLines = compactTraitLines(m_slot);
        constexpr qreal traitHeight = 20.0;
        constexpr qreal traitGap = 4.0;
        qreal traitY = imageRect.bottom() - 18.0
            - traitLines.size() * traitHeight
            - qMax(0, traitLines.size() - 1) * traitGap;
        for (const QString& trait : traitLines) {
            QFont traitFont = painter->font();
            traitFont.setPointSize(8);
            traitFont.setBold(true);
            painter->setFont(traitFont);
            const qreal traitWidth = qMin<qreal>(imageRect.width() - 20.0,
                                                 painter->fontMetrics().horizontalAdvance(trait) + 18.0);
            const QRectF traitRect(imageRect.left() + 9.0, traitY, traitWidth, traitHeight);
            painter->setPen(QPen(QColor(218, 190, 91), 1.2));
            painter->setBrush(QColor(24, 27, 35, 220));
            painter->drawRoundedRect(traitRect, 5, 5);
            painter->setPen(QColor(245, 232, 176));
            painter->drawText(traitRect.adjusted(7, 0, -7, 0), Qt::AlignVCenter | Qt::AlignLeft, trait);
            traitY += traitHeight + traitGap;
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(169, 116, 18, 220));
        painter->drawRoundedRect(bottomRect, 5, 5);
        QLinearGradient bottomGlow(bottomRect.topLeft(), bottomRect.bottomLeft());
        bottomGlow.setColorAt(0.0, QColor(239, 190, 55, 105));
        bottomGlow.setColorAt(1.0, QColor(119, 72, 7, 125));
        painter->fillRect(bottomRect.adjusted(1, 1, -1, -1), bottomGlow);

        painter->setPen(Qt::white);
        QFont nameFont = painter->font();
        nameFont.setPointSize(12);
        nameFont.setBold(true);
        painter->setFont(nameFont);
        QFontMetrics nameMetrics(nameFont);
        const QString heroName = nameMetrics.elidedText(m_slot.heroName, Qt::ElideRight,
                                                        qRound(bottomRect.width() - 74.0));
        painter->drawText(bottomRect.adjusted(10, 5, -66, -30), Qt::AlignLeft | Qt::AlignVCenter,
                          heroName);

        QFont localizedFont = painter->font();
        localizedFont.setPointSize(9);
        localizedFont.setBold(false);
        painter->setFont(localizedFont);
        QFontMetrics localizedMetrics(localizedFont);
        const QString localizedName = localizedMetrics.elidedText(heroLocalizedName(m_slot.heroType),
                                                                  Qt::ElideRight,
                                                                  qRound(bottomRect.width() - 74.0));
        painter->drawText(bottomRect.adjusted(10, 28, -66, -6), Qt::AlignLeft | Qt::AlignVCenter,
                          localizedName);

        painter->setPen(QColor(255, 245, 197));
        QFont costFont = painter->font();
        costFont.setPointSize(11);
        costFont.setBold(true);
        painter->setFont(costFont);
        painter->drawText(bottomRect.adjusted(0, 5, -10, -5), Qt::AlignVCenter | Qt::AlignRight,
                          QString("cost %1").arg(m_slot.cost));

        if (!m_canBuy) {
            painter->fillRect(rect(), QColor(0, 0, 0, 85));
            painter->setPen(QColor(240, 205, 140));
            QFont lockFont = painter->font();
            lockFont.setPointSize(12);
            lockFont.setBold(true);
            painter->setFont(lockFont);
            painter->drawText(imageRect.adjusted(12, 12, -12, -12),
                              Qt::AlignTop | Qt::AlignRight,
                              "不可购买");
        }
    }

    void paintDetails(QPainter* painter)
    {
        paintFrame(painter, QColor(128, 145, 188));

        const QRectF content = rect().adjusted(10, 8, -10, -8);
        const QRect back = backButtonRect();
        painter->setPen(QPen(QColor(202, 210, 234), 1.2));
        painter->setBrush(QColor(48, 53, 68));
        painter->drawRoundedRect(back, 5, 5);
        painter->setPen(QColor(240, 242, 248));
        QFont backFont = painter->font();
        backFont.setPointSize(8);
        backFont.setBold(true);
        painter->setFont(backFont);
        painter->drawText(back, Qt::AlignCenter, "返回");

        painter->setPen(QColor(244, 211, 122));
        QFont titleFont = painter->font();
        titleFont.setPointSize(10);
        titleFont.setBold(true);
        painter->setFont(titleFont);
        painter->drawText(QRectF(content.left(), content.top(), content.width() - 54, 22),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          QString("%1 %2").arg(m_slot.heroName, heroLocalizedName(m_slot.heroType)));

        const QString details = QString("费用 %1  星级 %2\n"
                                        "站位 %3\n"
                                        "HP %4  ATK %5\n"
                                        "Range %6  Mana %7/%8\n"
                                        "Armor %9  MR %10\n"
                                        "AS %11  Crit %12%\n"
                                        "Origin: %13\n"
                                        "Role: %14\n\n%15")
            .arg(m_slot.cost)
            .arg(m_slot.star)
            .arg(positionTypeText(m_slot.positionType))
            .arg(m_slot.maxHp)
            .arg(m_slot.atk)
            .arg(m_slot.range)
            .arg(m_slot.mana)
            .arg(m_slot.maxMana)
            .arg(m_slot.armor)
            .arg(m_slot.magicResist)
            .arg(m_slot.attackSpeed, 0, 'f', 2)
            .arg(m_slot.critRate * 100.0, 0, 'f', 0)
            .arg(originsText(m_slot))
            .arg(rolesText(m_slot))
            .arg(m_slot.skillDescription);

        painter->setPen(QColor(230, 232, 240));
        QFont bodyFont = painter->font();
        bodyFont.setPointSize(8);
        bodyFont.setBold(false);
        painter->setFont(bodyFont);
        painter->drawText(content.adjusted(0, 32, 0, 0),
                          Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                          details);
    }

    ShopSlot m_slot{};
    bool m_hasSlot = false;
    bool m_canBuy = false;
    bool m_alreadyOwned = false;
    bool m_detailMode = false;
    bool m_pressed = false;
    bool m_ignoreNextContextMenu = false;
    QPixmap m_portrait;
};
}

GameWindow::GameWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QVBoxLayout())
    , m_contentLayout(new QHBoxLayout())
    , m_view(new ZoomableGraphicsView(this))
    , m_shopScrollArea(new QScrollArea(this))
    , m_shopPanel(new QWidget())
    , m_topRoundLabel(new QLabel(this))
    , m_shopToggleButton(new QPushButton("Shop ▼", this))
    , m_enemyStatusLabel(new QLabel(this))
    , m_synergyStatusLabel(new QLabel(this))
    , m_shopTitleLabel(new QLabel("Shop", this))
    , m_shopCards{new ShopCardButton(this), new ShopCardButton(this), new ShopCardButton(this), new ShopCardButton(this), new ShopCardButton(this)}
    , m_refreshShopButton(new QPushButton("Refresh 4 gold", this))
    , m_resetButton(new QPushButton("Reset", this))
    , m_saveButton(new QPushButton("存档", this))
    , m_loadButton(new QPushButton("读取", this))
    , m_combatButton(new QPushButton("Start Combat", this))
    , m_hpStatusLabel(new QLabel(this))
    , m_goldStatusLabel(new QLabel(this))
    , m_levelStatusButton(new QPushButton(this))
    , m_roundStatusLabel(new QLabel(this))
    , m_phaseStatusLabel(new QLabel(this))
    , m_game(std::make_unique<Game>())
{
    setupUI();
    m_game->initialize();
    refreshStatusBar();
    refreshShopPanel();
    fitSceneInView();
    QTimer::singleShot(0, this, [this]() {
        fitSceneInView();
    });
}

GameWindow::~GameWindow()
{
    if (m_view != nullptr) {
        m_view->setScene(nullptr);
    }
}

void GameWindow::onResetButtonClicked() const
{
    if (m_game) {
        m_game->reset();
        refreshStatusBar();
        refreshShopPanel();
        fitSceneInView();
    }
}

void GameWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    fitSceneInView();
}

void GameWindow::setupUI()
{
    setCentralWidget(m_centralWidget);
    m_centralWidget->setLayout(m_mainLayout);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2b2b;
        }
        QWidget {
            background-color: #2b2b2b;
            color: #f2f2f2;
        }
        QPushButton {
            background-color: #2f2f2f;
            color: #f2f2f2;
            border: 1px solid #565656;
            border-radius: 4px;
            padding: 6px 14px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #3a3a3a;
        }
        QPushButton:pressed {
            background-color: #242424;
        }
    )");

    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setAlignment(Qt::AlignCenter);
    m_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_view->setMouseTracking(true);
    m_view->viewport()->setMouseTracking(true);

    const QString headerLabelStyle = R"(
        QLabel {
            background-color: #202126;
            color: #f2f2f2;
            border: 1px solid #4a4d58;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 700;
        }
    )";
    auto topBar = std::make_unique<QWidget>();
    auto topLayout = std::make_unique<QHBoxLayout>(topBar.get());
    auto* topBarLayout = topLayout.get();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);
    m_enemyStatusLabel->setStyleSheet(headerLabelStyle);
    m_enemyStatusLabel->setAlignment(Qt::AlignCenter);
    m_enemyStatusLabel->setFixedWidth(360);
    m_synergyStatusLabel->setStyleSheet("font-size: 12px; color: #d7c47b; font-weight: 600;");
    m_synergyStatusLabel->setAlignment(Qt::AlignCenter);
    m_synergyStatusLabel->setMinimumWidth(260);
    m_topRoundLabel->setStyleSheet("font-size: 18px; font-weight: 800; color: #f0d37a;");
    m_topRoundLabel->setAlignment(Qt::AlignCenter);
    topBarLayout->addWidget(m_enemyStatusLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_synergyStatusLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(m_topRoundLabel);
    topBarLayout->addStretch();
    releaseToQt(topLayout);
    m_mainLayout->addWidget(topBar.release());

    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(12);
    m_contentLayout->addWidget(m_view, 1);

    auto shopLayout = std::make_unique<QHBoxLayout>(m_shopPanel);
    auto* shopPanelLayout = shopLayout.get();
    shopLayout->setContentsMargins(12, 10, 12, 10);
    shopLayout->setSpacing(12);
    m_shopPanel->setObjectName("ShopPanel");
    m_shopPanel->setStyleSheet(R"(
        QWidget#ShopPanel {
            background-color: #202126;
            border: 1px solid #3f4350;
        }
        QLabel {
            border: none;
            color: #f2f2f2;
        }
    )");
    auto shopHeader = std::make_unique<QWidget>();
    auto shopHeaderLayout = std::make_unique<QVBoxLayout>(shopHeader.get());
    auto* shopHeaderItems = shopHeaderLayout.get();
    shopHeaderLayout->setContentsMargins(0, 0, 0, 0);
    shopHeaderLayout->setSpacing(10);
    m_shopTitleLabel->setAlignment(Qt::AlignCenter);
    m_shopTitleLabel->setStyleSheet("font-size: 18px; font-weight: 800; color: #f0d37a;");
    shopHeaderItems->addWidget(m_shopTitleLabel);
    m_refreshShopButton->setMinimumHeight(42);
    shopHeaderItems->addWidget(m_refreshShopButton);
    shopHeaderItems->addStretch();
    releaseToQt(shopHeaderLayout);
    shopPanelLayout->addWidget(shopHeader.release());

    for (int i = 0; i < static_cast<int>(m_shopCards.size()); ++i) {
        QPushButton* card = m_shopCards[i];
        card->setObjectName("ShopCard");
        card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        card->setFlat(true);
        shopPanelLayout->addWidget(card);
    }

    shopPanelLayout->addStretch(1);
    releaseToQt(shopLayout);
    m_shopScrollArea->setWidget(m_shopPanel);
    m_shopScrollArea->setWidgetResizable(true);
    m_shopScrollArea->setFrameShape(QFrame::NoFrame);
    m_shopScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_shopScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_shopScrollArea->setFixedHeight(kShopPanelExpandedHeight);
    m_shopScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_mainLayout->addLayout(m_contentLayout, 1);
    m_mainLayout->addWidget(m_shopScrollArea);

    auto controlBar = std::make_unique<QWidget>();
    auto controlLayout = std::make_unique<QHBoxLayout>(controlBar.get());
    auto* controls = controlLayout.get();
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(10);
    m_resetButton->setFixedWidth(86);
    controls->addWidget(m_resetButton);
    m_saveButton->setFixedWidth(72);
    m_loadButton->setFixedWidth(72);
    controls->addWidget(m_saveButton);
    controls->addWidget(m_loadButton);
    controls->addSpacing(14);
    m_combatButton->setFixedWidth(128);
    controls->addWidget(m_combatButton);
    controls->addSpacing(14);

    const QString statusStyle = R"(
        QLabel {
            background-color: #202126;
            color: #f2f2f2;
            border: 1px solid #4a4d58;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 600;
        }
    )";
    for (QLabel* label : {m_hpStatusLabel, m_goldStatusLabel}) {
        label->setStyleSheet(statusStyle);
        label->setAlignment(Qt::AlignCenter);
        controls->addWidget(label);
    }
    m_hpStatusLabel->setFixedWidth(104);
    m_goldStatusLabel->setFixedWidth(126);
    m_levelStatusButton->setStyleSheet(R"(
        QPushButton {
            background-color: #202126;
            color: #f2f2f2;
            border: 1px solid #4a4d58;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #2a2d35;
            border-color: #7a8194;
        }
        QPushButton:pressed {
            background-color: #181a1f;
        }
    )");
    m_levelStatusButton->setFixedWidth(112);
    m_levelStatusButton->setStyleSheet(statusButtonStyle(false));
    controls->addWidget(m_levelStatusButton);
    for (QLabel* label : {m_roundStatusLabel, m_phaseStatusLabel}) {
        label->setStyleSheet(statusStyle);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedWidth(112);
        controls->addWidget(label);
    }
    m_shopToggleButton->setFixedWidth(96);
    m_shopToggleButton->setStyleSheet(statusButtonStyle(true));
    m_shopToggleButton->setToolTip("收起商店栏");
    controls->addWidget(m_shopToggleButton);

    controls->addStretch();
    releaseToQt(controlLayout);
    m_mainLayout->addWidget(controlBar.release());

    connect(m_resetButton, &QPushButton::clicked,
            this, &GameWindow::onResetButtonClicked);
    connect(m_saveButton, &QPushButton::clicked,
            this, [this]() {
                if (m_game == nullptr) {
                    return;
                }
                // 采用固定单存档槽，避免存档与读档误选到不同 JSON 文件。
                const QString path = singleSaveSlotPath();
                QString error;
                if (!m_game->saveGame(path, &error)) {
                    QMessageBox::warning(this, "存档失败", error);
                    return;
                }
                QMessageBox::information(this, "存档成功",
                    QString("当前游戏状态已写入：\n%1").arg(path));
            });
    connect(m_loadButton, &QPushButton::clicked,
            this, [this]() {
                if (m_game == nullptr) {
                    return;
                }
                const QString path = singleSaveSlotPath();
                QString error;
                if (!m_game->loadGame(path, &error)) {
                    QMessageBox::warning(this, "读取失败",
                        QString("%1\n\n存档路径：\n%2").arg(error, path));
                    return;
                }
                refreshStatusBar();
                refreshShopPanel();
                fitSceneInView();
                QMessageBox::information(this, "读取成功", "存档状态已恢复。");
            });
    connect(m_combatButton, &QPushButton::clicked,
            this, [this]() {
                if (m_game == nullptr) {
                    return;
                }

                // 玩家只能开始战斗；战斗会在一方全灭时自动结算，不能手动结束。
                if (m_game->canStartCombat()) {
                    m_game->startCombat();
                }
            });
    connect(m_game.get(), &Game::stateChanged,
            this, [this]() {
                refreshStatusBar();
                refreshShopPanel();
            });
    connect(m_game.get(), &Game::unitCardRequested,
            this, &GameWindow::showUnitCard);
    connect(m_game.get(), &Game::gameFinished,
            this, &GameWindow::showGameResult);
    connect(m_levelStatusButton, &QPushButton::clicked,
            this, [this]() {
                if (m_game) {
                    m_game->buyLevelProgress();
                }
            });
    connect(m_shopToggleButton, &QPushButton::clicked,
            this, [this]() {
                setShopPanelExpanded(!m_shopPanelExpanded);
            });
    for (int i = 0; i < static_cast<int>(m_shopCards.size()); ++i) {
        connect(m_shopCards[i], &QPushButton::clicked,
                this, [this, i]() {
                    if (m_game) {
                        m_game->buyShopUnit(i);
                    }
                });
    }
    connect(m_refreshShopButton, &QPushButton::clicked,
            this, [this]() {
                if (m_game) {
                    m_game->refreshShop();
                }
            });
    connect(m_game.get(), &Game::equipmentAtlasRequested,
            this, &GameWindow::showEquipmentAtlas);

    m_view->setScene(m_game->scene());
}

void GameWindow::fitSceneInView() const
{
    if (!m_view || !m_game || !m_game->scene()) {
        return;
    }

    const QRectF bounds = m_game->scene()->sceneRect();
    if (bounds.isNull()) {
        return;
    }

    constexpr qreal kMaxAutoScale = 1.80;

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const QRectF paddedBounds = bounds.adjusted(-14, -14, 14, 14);
    const QSize viewportSize = m_view->viewport()->size();
    if (viewportSize.isEmpty() || paddedBounds.width() <= 0.0 || paddedBounds.height() <= 0.0) {
        return;
    }

    const qreal scaleX = static_cast<qreal>(viewportSize.width()) / paddedBounds.width();
    const qreal scaleY = static_cast<qreal>(viewportSize.height()) / paddedBounds.height();
    // Startup fit must show the whole scene. Do not force a minimum scale here;
    // otherwise compact windows crop the top/bottom and immediately show scroll bars.
    const qreal scaleFactor = qMin(qMin(scaleX, scaleY), kMaxAutoScale);

    m_view->resetTransform();
    m_view->scale(scaleFactor, scaleFactor);
    m_view->centerOn(bounds.center());
}

void GameWindow::setShopPanelExpanded(bool expanded)
{
    if (m_shopScrollArea == nullptr || m_shopToggleButton == nullptr) {
        return;
    }

    m_shopPanelExpanded = expanded;
    m_shopToggleButton->setText(expanded ? "Shop ▼" : "Shop ▲");
    m_shopToggleButton->setToolTip(expanded ? "收起商店栏" : "从底部打开商店栏");

    const int targetHeight = expanded ? kShopPanelExpandedHeight : 0;
    const int startHeight = m_shopScrollArea->height();
    if (m_shopPanelAnimation != nullptr) {
        m_shopPanelAnimation->stop();
        m_shopPanelAnimation->deleteLater();
        m_shopPanelAnimation = nullptr;
    }

    if (startHeight == targetHeight) {
        m_shopScrollArea->setFixedHeight(targetHeight);
        fitSceneInView();
        return;
    }

    auto* animation = new QVariantAnimation(this);
    m_shopPanelAnimation = animation;
    animation->setDuration(kShopPanelAnimationMs);
    animation->setStartValue(startHeight);
    animation->setEndValue(targetHeight);
    animation->setEasingCurve(expanded ? QEasingCurve::OutCubic : QEasingCurve::InCubic);
    connect(animation, &QVariantAnimation::valueChanged,
            this, [this](const QVariant& value) {
                if (m_shopScrollArea != nullptr) {
                    m_shopScrollArea->setFixedHeight(value.toInt());
                }
            });
    connect(animation, &QVariantAnimation::finished,
            this, [this, animation, targetHeight]() {
                if (m_shopScrollArea != nullptr) {
                    m_shopScrollArea->setFixedHeight(targetHeight);
                }
                if (m_shopPanelAnimation == animation) {
                    m_shopPanelAnimation = nullptr;
                }
                animation->deleteLater();
                fitSceneInView();
            });
    animation->start();
}

void GameWindow::refreshStatusBar() const
{
    if (!m_game) {
        return;
    }

    const Player& player = m_game->player();
    const Player& enemy = m_game->enemy();
    m_topRoundLabel->setText(QString("Round %1").arg(player.round()));
    m_enemyStatusLabel->setText(QString("Enemy  HP %1   Gold %2   Lv %3  %4/%5")
        .arg(enemy.hp())
        .arg(enemy.gold())
        .arg(enemy.level())
        .arg(enemy.levelProgress())
        .arg(enemy.levelProgressNeeded()));
    const QStringList synergies = m_game->activePlayerSynergyTexts();
    m_synergyStatusLabel->setText(synergies.isEmpty()
        ? "羁绊：未激活"
        : "羁绊：" + synergies.join(" | "));
    m_hpStatusLabel->setText(QString("HP %1").arg(player.hp()));
    m_goldStatusLabel->setText(QString("Gold %1").arg(player.gold()));
    if (player.levelProgressNeeded() > 0) {
        m_levelStatusButton->setText(QString("Lv %1  %2/%3")
            .arg(player.level())
            .arg(player.levelProgress())
            .arg(player.levelProgressNeeded()));
    } else {
        m_levelStatusButton->setText(QString("Lv %1  MAX").arg(player.level()));
    }
    m_levelStatusButton->setStyleSheet(statusButtonStyle(
        !m_game->isGameOver() && player.gold() >= 4 && player.levelProgressNeeded() > 0));
    m_levelStatusButton->setEnabled(!m_game->isGameOver() && player.levelProgressNeeded() > 0);
    m_roundStatusLabel->setText(QString("Round %1").arg(player.round()));
    if (m_game->isGameOver()) {
        m_phaseStatusLabel->setText("Game Over");
    } else if (m_game->phase() == GamePhase::Combat) {
        m_phaseStatusLabel->setText("Combat");
    } else {
        const int remainingSeconds = m_game->preparationRemainingSeconds();
        const int minutes = remainingSeconds / 60;
        const int seconds = remainingSeconds % 60;
        m_phaseStatusLabel->setText(QString("Prep %1:%2")
            .arg(minutes)
            .arg(seconds, 2, 10, QChar('0')));
    }
    if (m_game->isGameOver()) {
        m_combatButton->setText("Game Over");
        m_combatButton->setEnabled(false);
    } else if (m_game->phase() == GamePhase::Combat) {
        m_combatButton->setText("Combat Running");
        m_combatButton->setEnabled(false);
    } else {
        m_combatButton->setText("Start Combat");
        m_combatButton->setEnabled(m_game->canStartCombat());
    }
}

void GameWindow::showEquipmentAtlas()
{
    QDialog dialog(this);
    dialog.setModal(true);
    dialog.setWindowTitle("装备图谱");
    dialog.setMinimumSize(930, 555);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #1c1e24;
            color: #f2f2f2;
        }
        QLabel {
            color: #d9dce5;
            font-size: 15px;
            background-color: transparent;
        }
        QLabel#AtlasTitle {
            color: #f0d37a;
            font-size: 25px;
            font-weight: 800;
        }
        QLabel#RecipeSymbol {
            color: #b4bac9;
            font-size: 22px;
            font-weight: 700;
        }
        QPushButton#EquipmentNode {
            background-color: #292d38;
            color: #f2f2f2;
            border: 1px solid #687085;
            border-radius: 5px;
            padding: 12px 10px;
            font-size: 13px;
            font-weight: 700;
            text-align: center;
        }
        QPushButton#EquipmentNode:hover {
            background-color: #343a49;
            border-color: #a8b0c7;
        }
        QPushButton#CombinedEquipmentNode {
            background-color: #373123;
            color: #f0d37a;
            border: 1px solid #a98844;
            border-radius: 5px;
            padding: 12px 10px;
            font-size: 13px;
            font-weight: 800;
        }
        QPushButton#CombinedEquipmentNode:hover {
            background-color: #473c27;
            border-color: #ddb65a;
        }
    )");

    auto layout = std::make_unique<QVBoxLayout>(&dialog);
    auto* atlasLayout = layout.get();
    layout->setContentsMargins(22, 18, 22, 20);
    layout->setSpacing(16);
    QLabel title("装备图谱", &dialog);
    title.setObjectName("AtlasTitle");
    atlasLayout->addWidget(&title);

    auto tree = std::make_unique<QGridLayout>();
    auto* recipeTree = tree.get();
    tree->setHorizontalSpacing(12);
    tree->setVerticalSpacing(13);
    tree->setColumnStretch(0, 2);
    tree->setColumnStretch(2, 2);
    tree->setColumnStretch(4, 3);

    auto createNode = [this, &dialog](EquipmentType type, bool combined) {
        auto button = std::make_unique<QPushButton>(equipmentName(type), &dialog);
        button->setObjectName(combined ? "CombinedEquipmentNode" : "EquipmentNode");
        button->setMinimumHeight(54);
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        button->setToolTip("右键查看装备属性与效果");
        connect(button.get(), &QPushButton::customContextMenuRequested,
                this, [this, type](const QPoint&) {
                    showEquipmentInfo(type);
                });
        return button;
    };
    auto symbol = [&dialog](const QString& text) {
        auto label = std::make_unique<QLabel>(text, &dialog);
        label->setObjectName("RecipeSymbol");
        label->setAlignment(Qt::AlignCenter);
        return label;
    };
    auto addRecipe = [&](int row, EquipmentType first, EquipmentType second, EquipmentType result) {
        recipeTree->addWidget(createNode(first, false).release(), row, 0);
        recipeTree->addWidget(symbol("+").release(), row, 1);
        recipeTree->addWidget(createNode(second, false).release(), row, 2);
        recipeTree->addWidget(symbol("->").release(), row, 3);
        recipeTree->addWidget(createNode(result, true).release(), row, 4);
    };

    // 图谱直接对应后端合成表，玩家可以从散件关系预判下一件成装。
    addRecipe(0, EquipmentType::RecurveBow, EquipmentType::NeedlesslyLargeRod,
              EquipmentType::GuinsoosRageblade);
    addRecipe(1, EquipmentType::BFSword, EquipmentType::TearOfTheGoddess,
              EquipmentType::SpearOfShojin);
    addRecipe(2, EquipmentType::ChainVest, EquipmentType::ChainVest,
              EquipmentType::Thornmail);
    addRecipe(3, EquipmentType::GiantsBelt, EquipmentType::GiantsBelt,
              EquipmentType::WarmogsArmor);

    auto noRecipe = std::make_unique<QLabel>("当前无合成关系", &dialog);
    noRecipe->setAlignment(Qt::AlignCenter);
    noRecipe->setStyleSheet("color: #949cad; font-size: 13px;");
    recipeTree->addWidget(createNode(EquipmentType::NegatronCloak, false).release(), 4, 0);
    recipeTree->addWidget(createNode(EquipmentType::SparringGloves, false).release(), 4, 2);
    recipeTree->addWidget(noRecipe.release(), 4, 4);
    recipeTree->addWidget(createNode(EquipmentType::IronSword, false).release(), 5, 0);
    recipeTree->addWidget(createNode(EquipmentType::HasteGloves, false).release(), 5, 2);
    recipeTree->addWidget(createNode(EquipmentType::BlueCrystal, false).release(), 5, 4);

    atlasLayout->addLayout(recipeTree, 1);
    releaseToQt(tree);
    releaseToQt(layout);
    dialog.exec();
}

void GameWindow::showEquipmentInfo(EquipmentType equipment)
{
    QDialog dialog(this);
    dialog.setModal(true);
    dialog.setWindowTitle(equipmentName(equipment));
    dialog.setMinimumSize(460, 260);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #202126;
            color: #f2f2f2;
        }
        QLabel {
            background-color: transparent;
            color: #ebedf3;
        }
        QPushButton {
            background-color: #2e323e;
            color: #f2f2f2;
            border: 1px solid #596174;
            border-radius: 4px;
            padding: 8px 24px;
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #3a4050;
        }
    )");

    auto layout = std::make_unique<QVBoxLayout>(&dialog);
    auto* equipmentLayout = layout.get();
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(14);
    QLabel title(equipmentName(equipment), &dialog);
    title.setStyleSheet("font-size: 21px; color: #f0d37a; font-weight: 800;");
    QLabel description(equipmentDescription(equipment), &dialog);
    description.setWordWrap(true);
    description.setAlignment(Qt::AlignTop);
    description.setStyleSheet("font-size: 15px; line-height: 1.4; background-color: #292b33; border: 1px solid #474c59; border-radius: 4px; padding: 14px;");
    QPushButton closeButton("Close", &dialog);
    connect(&closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    equipmentLayout->addWidget(&title);
    equipmentLayout->addWidget(&description, 1);
    equipmentLayout->addWidget(&closeButton, 0, Qt::AlignRight);
    releaseToQt(layout);
    dialog.exec();
}

void GameWindow::showUnitCard(const Unit* unit)
{
    if (unit == nullptr || m_game == nullptr) {
        return;
    }

    const int unitId = unit->id();
    QDialog dialog(this);
    dialog.setWindowTitle(unit->name());
    dialog.setModal(true);
    dialog.setMinimumSize(560, 520);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #202126;
            color: #f2f2f2;
        }
        QLabel {
            color: #f2f2f2;
        }
        QPushButton {
            background-color: #3a2c2c;
            color: #f2f2f2;
            border: 1px solid #9d6a5f;
            border-radius: 4px;
            padding: 8px 12px;
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #523535;
        }
    )");

    auto layout = std::make_unique<QVBoxLayout>(&dialog);
    auto* unitCardLayout = layout.get();
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(12);
    QLabel titleLabel(QString("%1  %2").arg(unit->name(), heroLocalizedName(unit->name())), &dialog);
    titleLabel.setStyleSheet("font-size: 22px; font-weight: 800; color: #f0d37a;");

    QLabel statsLabel(unitStatsText(unit), &dialog);
    statsLabel.setTextFormat(Qt::RichText);
    statsLabel.setStyleSheet("font-size: 16px; font-family: Menlo, Monaco, monospace;");

    QLabel traitsLabel(QString("Origin: %1\nRole: %2").arg(originsText(unit), rolesText(unit)), &dialog);
    traitsLabel.setWordWrap(true);
    traitsLabel.setStyleSheet("font-size: 14px; color: #d6d9e5;");

    QLabel equipmentLabel(equipmentText(*unit), &dialog);
    equipmentLabel.setWordWrap(true);
    equipmentLabel.setStyleSheet("font-size: 14px; color: #f0d37a; font-weight: 700;");

    QLabel combatStatusLabel(m_game->combatStatusText(unit), &dialog);
    combatStatusLabel.setWordWrap(true);
    combatStatusLabel.setVisible(!combatStatusLabel.text().isEmpty());
    combatStatusLabel.setStyleSheet("font-size: 14px; color: #8fd1ff; font-weight: 700;");

    QLabel skillLabel(unitSkillText(unit), &dialog);
    skillLabel.setTextFormat(Qt::RichText);
    skillLabel.setWordWrap(true);
    skillLabel.setAlignment(Qt::AlignTop);
    skillLabel.setMinimumHeight(170);
    skillLabel.setStyleSheet(R"(
        QLabel {
            background-color: #292b33;
            border: 1px solid #4a4d58;
            border-radius: 4px;
            padding: 12px;
            font-size: 14px;
            color: #e8eaf2;
        }
    )");

    QPushButton sellButton(QString("Sell for %1 gold").arg(unit->cost()), &dialog);
    connect(&sellButton, &QPushButton::clicked, &dialog, [this, unitId, &dialog]() {
        if (m_game != nullptr && m_game->sellUnitById(unitId)) {
            dialog.accept();
        }
    });

    unitCardLayout->addWidget(&titleLabel);
    unitCardLayout->addWidget(&statsLabel);
    unitCardLayout->addWidget(&traitsLabel);
    unitCardLayout->addWidget(&equipmentLabel);
    unitCardLayout->addWidget(&combatStatusLabel);
    unitCardLayout->addWidget(&skillLabel, 1);
    unitCardLayout->addWidget(&sellButton);
    releaseToQt(layout);
    dialog.exec();
}

void GameWindow::showGameResult(bool playerWon)
{
    QDialog dialog(this);
    dialog.setModal(true);
    dialog.setWindowTitle(playerWon ? "Victory" : "Defeat");
    dialog.setFixedSize(520, 280);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #171a20;
            border: 2px solid #b89543;
        }
        QLabel {
            background-color: transparent;
        }
        QPushButton {
            background-color: #2a303b;
            color: #ede2c3;
            border: 1px solid #8e7642;
            border-radius: 4px;
            padding: 9px 30px;
            font-size: 14px;
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #353c48;
        }
    )");

    auto layout = std::make_unique<QVBoxLayout>(&dialog);
    auto* resultLayout = layout.get();
    layout->setContentsMargins(28, 24, 28, 22);
    resultLayout->addStretch();

    QLabel title(playerWon ? "VICTORY" : "DEFEAT", &dialog);
    title.setAlignment(Qt::AlignCenter);
    title.setStyleSheet(playerWon
        ? "color: #d9bb65; font-size: 54px; font-weight: 800; font-style: italic; font-family: 'Snell Roundhand', 'Times New Roman';"
        : "color: #c76c65; font-size: 54px; font-weight: 800; font-style: italic; font-family: 'Snell Roundhand', 'Times New Roman';");
    resultLayout->addWidget(&title);

    QLabel subtitle(playerWon ? "ENEMY DEFEATED" : "YOUR TACTICIANS HAVE FALLEN", &dialog);
    subtitle.setAlignment(Qt::AlignCenter);
    subtitle.setStyleSheet("color: #b6bac5; font-size: 13px; letter-spacing: 2px;");
    resultLayout->addWidget(&subtitle);
    resultLayout->addStretch();

    QPushButton closeButton("Close", &dialog);
    connect(&closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    resultLayout->addWidget(&closeButton, 0, Qt::AlignCenter);
    releaseToQt(layout);
    dialog.exec();
}

void GameWindow::refreshShopPanel() const
{
    if (!m_game) {
        return;
    }

    const std::vector<ShopSlot>& shopSlots = m_game->shopSlots();
    const Player& player = m_game->player();
    const bool benchIsFull = m_game->benchFull();
    for (int i = 0; i < static_cast<int>(m_shopCards.size()); ++i) {
        auto* card = static_cast<ShopCardButton*>(m_shopCards[i]);
        if (i < static_cast<int>(shopSlots.size())) {
            const ShopSlot& slot = shopSlots[i];
            const bool canBuy = !m_game->isGameOver() && !benchIsFull && player.gold() >= slot.cost;
            const bool alreadyOwned = m_game->playerOwnsHero(slot.heroName);
            card->setVisible(true);
            card->setShopSlot(&slot, canBuy, alreadyOwned);
        } else {
            card->setVisible(true);
            card->setShopSlot(nullptr, false, false);
        }
    }
    m_refreshShopButton->setEnabled(!m_game->isGameOver() && player.gold() >= 4);
    m_refreshShopButton->setStyleSheet(statusButtonStyle(!m_game->isGameOver() && player.gold() >= 4));
}
