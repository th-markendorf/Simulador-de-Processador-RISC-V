#ifndef LAUNCHERWINDOW_H
#define LAUNCHERWINDOW_H

#include <QMainWindow> // Use QMainWindow se for sua janela principal
#include <memory>
#include "core/Core.h"
#include "mainwindow.h"
#include "demowindow.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class LauncherWindow;
}

QT_END_NAMESPACE

class LauncherWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LauncherWindow(QWidget *parent = nullptr);
    ~LauncherWindow();
private slots:
    void on_simuladorButton_clicked();
    void on_demoButton_clicked();

private:
    Ui::LauncherWindow *ui;

    // O Launcher VAI POSSUIR o Core
    std::unique_ptr<Core> m_core;

    // Ponteiros para as janelas (para que não abram várias)
    MainWindow* m_mainWindow;
    DemoWindow* m_demoWindow;
};

#endif // LAUNCHERWINDOW_H