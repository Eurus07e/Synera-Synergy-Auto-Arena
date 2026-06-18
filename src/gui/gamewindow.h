#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <array>
#include <memory>

class Game;
class Unit;
class QGraphicsView;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QScrollArea;
class QVariantAnimation;
enum class EquipmentType;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onResetButtonClicked() const;

private:
    void setupUI();
    void fitSceneInView() const;
    void refreshStatusBar() const;
    void refreshShopPanel() const;
    void setShopPanelExpanded(bool expanded);
    void showUnitCard(const Unit* unit);
    void showGameResult(bool playerWon);
    void showEquipmentAtlas();
    void showEquipmentInfo(EquipmentType equipment);

    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_contentLayout;
    QGraphicsView* m_view;
    QScrollArea* m_shopScrollArea;
    QWidget* m_shopPanel;
    QLabel* m_topRoundLabel;
    QPushButton* m_shopToggleButton;
    QLabel* m_enemyStatusLabel;
    QLabel* m_synergyStatusLabel;
    QLabel* m_shopTitleLabel;
    std::array<QPushButton*, 5> m_shopCards;
    QPushButton* m_refreshShopButton;
    QPushButton* m_resetButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_combatButton;
    QLabel* m_hpStatusLabel;
    QLabel* m_goldStatusLabel;
    QPushButton* m_levelStatusButton;
    QLabel* m_roundStatusLabel;
    QLabel* m_phaseStatusLabel;
    QVariantAnimation* m_shopPanelAnimation = nullptr;
    bool m_shopPanelExpanded = true;
    std::unique_ptr<Game> m_game;
};

#endif // GAMEWINDOW_H
