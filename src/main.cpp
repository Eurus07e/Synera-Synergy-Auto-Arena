#include <QApplication>
#include <QCoreApplication>
#include "gui/gamewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Synera Starter");
    QCoreApplication::setApplicationVersion("1.0");

    GameWindow window;
    window.setWindowTitle("Synera - Starter");
    window.showFullScreen();

    return QApplication::exec();
}
