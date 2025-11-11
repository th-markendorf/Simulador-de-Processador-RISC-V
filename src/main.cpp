#include <QApplication>

#include "gui/launcherwindow.h"
#include "gui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LauncherWindow w; // <--- Alterado (Inicia o Launcher)
    w.show();
    return a.exec();
}