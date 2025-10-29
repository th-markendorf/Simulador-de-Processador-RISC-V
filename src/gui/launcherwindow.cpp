#include "launcherwindow.h"
#include "ui_launcherwindow.h"

LauncherWindow::LauncherWindow(QWidget *parent) : 
    QMainWindow(parent), 
    ui(new Ui::LauncherWindow), 
    m_core(std::make_unique<Core>(1024 * 1024)), // O Launcher cria o Core!
    m_mainWindow(nullptr),
    m_demoWindow(nullptr) {
    ui->setupUi(this);
}

LauncherWindow::~LauncherWindow() 
{
    delete ui;
}

// Abre a MainWindow (Simulador)
void LauncherWindow::on_simuladorButton_clicked() 
{
    if (!m_mainWindow) {
        // Passa o ponteiro bruto (m_core.get()) para a MainWindow
        m_mainWindow = new MainWindow(m_core.get());
    }
    m_mainWindow->show();
}

// Abre a DemoWindow (Demonstrador)
void LauncherWindow::on_demoButton_clicked() 
{
    if (!m_demoWindow) {
        m_demoWindow = new DemoWindow(m_core.get());
    }
    m_demoWindow->show();
}
