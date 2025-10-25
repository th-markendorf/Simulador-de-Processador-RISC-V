#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include <QTableWidget>

#include "core/Core.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Core* core, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_stepButton_clicked();
    void on_runButton_clicked();
    void on_resetButton_clicked();
    void on_loadButton_clicked();

private:
    Ui::MainWindow *ui;

    void updateUI(); // Função helper
    void on_run_timer_timeout();
    void loadProgramFromFile(const QString& filePath);

    Core* m_core;
    QTimer *m_runTimer;
};

#endif // MAINWINDOW_H