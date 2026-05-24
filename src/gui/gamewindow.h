#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <array>

class Game;
class Unit;
class QGraphicsView;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QScrollArea;
enum class EquipmentType;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onResetButtonClicked();

private:
    void setupUI();
    void fitSceneInView();
    void refreshStatusBar();
    void refreshShopPanel();
    void showUnitCard(Unit* unit);
    void showGameResult(bool playerWon);
    void showEquipmentAtlas();
    void showEquipmentInfo(EquipmentType equipment);

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    QGraphicsView* m_view;
    QPushButton* m_equipmentAtlasButton;
    QScrollArea* m_shopScrollArea;
    QWidget* m_shopPanel;
    QLabel* m_topRoundLabel;
    QLabel* m_enemyStatusLabel;
    QLabel* m_synergyStatusLabel;
    QLabel* m_shopTitleLabel;
    std::array<QPushButton*, 5> m_shopCards;
    std::array<QLabel*, 5> m_shopNameLabels;
    std::array<QLabel*, 5> m_shopMetaLabels;
    std::array<QLabel*, 5> m_shopStatsLabels;
    std::array<QLabel*, 5> m_shopTraitsLabels;
    std::array<QLabel*, 5> m_shopSkillLabels;
    QPushButton* m_refreshShopButton;
    QPushButton* m_resetButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_combatButton;
    QLabel* m_hpStatusLabel;
    QLabel* m_goldStatusLabel;
    QPushButton* m_levelStatusButton;
    QLabel* m_roundStatusLabel;
    Game* m_game;
};

#endif // GAMEWINDOW_H
