#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <iomanip>

// O construtor cria a janela e inicializa seu Core
MainWindow::MainWindow(Core* core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_core(core) // Armazena o ponteiro recebido
{
    ui->setupUi(this);

    m_runTimer = new QTimer(this);
    connect(m_runTimer, &QTimer::timeout, this, &MainWindow::on_run_timer_timeout);

    // Deixa as caixas de texto como "somente leitura"
    ui->logView->setReadOnly(true);
    ui->registersView->setReadOnly(true);

    // Carrega o programa padrão ao iniciar
    on_loadButton_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Opção 1 (Executar Programa)
 */
void MainWindow::on_loadButton_clicked()
{
    m_runTimer->stop(); // Para a simulação se estiver rodando
    ui->runButton->setText("Run");
    ui->runButton->setEnabled(true);
    ui->stepButton->setEnabled(true);

    m_core->reset(); // Reseta o processador

    std::vector<uint32_t> programa_exemplo = {
        0xFB000313, // addi x6, x0, -80
        0x00435393, // slti x7, x6, 4
        0x40435413, // xori x8, x6, 1028
        0x00000000  // Instrução Nula (para finalizar)
    };
    m_core->load_program(programa_exemplo); // Carrega o programa

    ui->logView->clear(); // Limpa o log
    ui->logView->append("Programa carregado. Core resetado.");
    updateUI(); // Atualiza a exibição dos registradores
}

/**
 * @brief  Opção 5 (Resetar Core)
 */
void MainWindow::on_resetButton_clicked()
{
    m_runTimer->stop();
    ui->runButton->setText("Run");
    m_core->reset(); // Chama o reset do core
    ui->logView->clear();
    ui->logView->append("Processador resetado.");
    updateUI(); // Atualiza a exibição
}

/**
 * @brief Opção 3 (Executar Step)
 */
void MainWindow::on_stepButton_clicked()
{
    // Chama a função step() do SEU Core
    std::string log_msg = m_core->step();

    // Coloca o log retornado na caixa de texto 'logView'
    ui->logView->append(QString::fromStdString(log_msg));

    // Atualiza os registradores na tela (substitui o imprimir_register())
    updateUI();

    // Se a simulação terminou, desativa os botões
    if (m_core->is_finished()) {
        m_runTimer->stop();
        ui->runButton->setText("Run");
        ui->runButton->setEnabled(false);
        ui->stepButton->setEnabled(false);
        ui->logView->append("Simulacao finalizada.");
    }
}

/**
 * @brief Opção 2 (Executar Run)
 * Esta é a principal diferença: em vez de chamar core.run() (que trava),
 * nós iniciamos um timer que clica no "Step" muito rápido.
 */
void MainWindow::on_runButton_clicked()
{
    if (m_runTimer->isActive()) {
        m_runTimer->stop();
        ui->runButton->setText("Run");
    } else {
        m_runTimer->start(50);
        ui->runButton->setText("Stop");
    }
}

/**
 * @brief Esta função é chamada pelo timer (a cada 50ms)
 * quando o "Run" está ativo.
 */
void MainWindow::on_run_timer_timeout()
{
    on_stepButton_clicked();
}

/**
 * @brief Opção 4 (Exibir Registradores)
 * Esta função é chamada automaticamente após cada ação.
 */
void MainWindow::updateUI()
{
    std::array<uint32_t, 32> regs = m_core->get_registradores();
    uint32_t pc = m_core->get_program_counter();

    // Formata o texto
    std::stringstream ss_regs;
    ss_regs << std::hex << std::setfill('0');

    for (int i = 0; i < 32; ++i) {
        ss_regs << "x" << std::setw(2) << std::dec << i << ": "
                << "0x" << std::setw(8) << std::hex << regs[i] << "\n";
    }

    ss_regs << "pc: "
            << "0x" << std::setw(8) << std::hex << pc << "\n";

    // Envia o texto formatado para a caixa 'registersView'
    ui->registersView->setText(QString::fromStdString(ss_regs.str()));
}