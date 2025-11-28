#include "launcherwindow.h"
#include "ui_launcherwindow.h"
#include <QMessageBox>
#include <QPushButton>

LauncherWindow::LauncherWindow(QWidget *parent) : 
    QMainWindow(parent), 
    ui(new Ui::LauncherWindow), 
    m_core(std::make_unique<Core>(1024 * 1024)),
    m_mainWindow(nullptr),
    m_demoWindow(nullptr) {

    ui->setupUi(this);

    // AJUSTE DA JANELA
    this->setWindowTitle("Simulador RISC-V");
    this->setFixedSize(400, 500);

    // Configurações de Layout
    int btnWidth = 220;
    int btnHeight = 50;
    int centerX = (this->width() - btnWidth) / 2;
    int startY = 140;
    int gap = 70;

    // REAJUSTE DOS BOTÕES EXISTENTES
    ui->simuladorButton->setGeometry(centerX, startY, btnWidth, btnHeight);
    ui->simuladorButton->setText("Simulador");

    ui->demoButton->setGeometry(centerX, startY + gap, btnWidth, btnHeight);
    ui->demoButton->setText("Modo Demo");

    //BOTÃO "SOBRE"
    QPushButton *btnSobre = new QPushButton("Sobre", this);
    btnSobre->setGeometry(centerX, startY + (gap * 2), btnWidth, btnHeight);

    // Conecta o clique à mensagem explicativa
    connect(btnSobre, &QPushButton::clicked, this, [this]() {
         QString texto =
             "<h3>Bem-vindo ao Simulador RISC-V</h3>"
             "<p>Este software é uma implementação didática de um processador baseada na arquitetura RISC-V (RV32I).</p>"
             "<hr>"
             "<h4>Modo 1: Simulador Completo (Pipeline)</h4>"
             "<p>Simula o comportamento real do processador executando programas inteiros. Destaques:</p>"
             "<ul>"
             "<li><b>Pipeline de 5 Estágios:</b> Visualize instruções fluindo simultaneamente por Busca (IF), Decodificação (ID), Execução (EX), Memória (MEM) e Escrita (WB).</li>"
             "<li><b>Depuração (Step-by-Step):</b> Execute ciclo a ciclo (<i>Step</i>) para entender como o processador resolve conflitos de dados e desvios.</li>"
             "<li><b>Inspeção de Memória e Cache:</b> Visualize o mapeamento da memória RAM e o comportamento de Hits/Misses na Cache de dados.</li>"
             "</ul>"
             "<hr>"
             "<h4>Modo 2: Laboratório Demo</h4>"
             "<p>Ambiente isolado para testes unitários de instruções:</p>"
             "<ul>"
             "<li>Teste operações matemáticas (ADD, SUB, MUL, DIV) isoladamente.</li>"
             "<li>Entenda a lógica da ULA definindo manualmente os operandos e imediatos.</li>"
             "<li>Visualize as mudanças nos registradores sem a complexidade do pipeline completo.</li>"
             "</ul>";

         QMessageBox::about(this, "Sobre o Projeto", texto);
    });
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
