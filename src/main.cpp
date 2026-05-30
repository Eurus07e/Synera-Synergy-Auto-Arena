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
    window.resize(900, 700);
    window.show();

    return QApplication::exec();
}
