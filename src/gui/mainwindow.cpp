#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <iomanip>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>

static const std::array<QString, 32> abiNames = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

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
    ui->registersTable->setColumnCount(4);
    ui->registersTable->setHorizontalHeaderLabels(QStringList() << "Reg" << "ABI" << "Hexadecimal" << "Decimal");
    ui->registersTable->setRowCount(33); // x0-x31 + pc

    for (int i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i, 0, new QTableWidgetItem(QString("x%1").arg(i)));
        ui->registersTable->setItem(i, 1, new QTableWidgetItem(abiNames[i]));
    }
    ui->registersTable->setItem(32, 0, new QTableWidgetItem("pc"));
    ui->registersTable->setItem(32, 1, new QTableWidgetItem("-"));

    ui->registersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->registersTable->setFixedWidth(435);

    ui->registersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->registersTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    m_core->reset(); // Garante que o core esteja zerado
    ui->logView->append("Simulador iniciado. Use 'Load' para carregar um programa.");
    updateUI(); // Atualiza a exibição (registradores zerados)

    // Desativa os botões de execução até que um programa seja carregado
    ui->runButton->setEnabled(false);
    ui->stepButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Opção 1 (Carregar Programa)
 */
void MainWindow::on_loadButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Abrir Programa Risc-V", // Título da Janela
        "", // Diretório inicial
        "Arquivos de Programa Hex (*.hex *.txt);;Todos os Arquivos (*)" // Filtros
    );

    // 2. Verifica se o usuário selecionou um arquivo
    if (filePath.isEmpty()) {
        ui->logView->append("[AVISO] Carregamento cancelado pelo usuario.");
        return; // Usuário cancelou
    }

    // 3. Chama a nova função helper para fazer o trabalho
    loadProgramFromFile(filePath);
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

    // 1. Formata e preenche os registradores (x0-x31)
    for (int i = 0; i < 32; ++i) {
        uint32_t valor = regs[i];

        // Coluna 2: Hexadecimal
        QString hexVal = QString("0x%1").arg(valor, 8, 16, QChar('0'));
        ui->registersTable->setItem(i, 2, new QTableWidgetItem(hexVal));

        // Coluna 3: Decimal
        // Converte para int32_t para exibir valores negativos corretamente
        QString decVal = QString::number(static_cast<int32_t>(valor));
        ui->registersTable->setItem(i, 3, new QTableWidgetItem(decVal));
    }

    // 2. Formata e preenche o PC (Linha 32)
    QString hexPC = QString("0x%1").arg(pc, 8, 16, QChar('0'));
    QString decPC = QString::number(pc);
    ui->registersTable->setItem(32, 2, new QTableWidgetItem(hexPC));
    ui->registersTable->setItem(32, 3, new QTableWidgetItem(decPC));
}

/**
 * @brief Função helper que lê um arquivo .hex e o carrega no Core.
 */
void MainWindow::loadProgramFromFile(const QString &filePath)
{
    std::vector<uint32_t> programa;
    QFile file(filePath);

    // 1. Tenta abrir o arquivo
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erro", "Nao foi possivel abrir o arquivo: " + file.errorString());
        return;
    }

    // 2. Lê o arquivo linha por linha
    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine().trimmed();

        // Ignora linhas vazias ou comentários (iniciados com # ou //)
        if (line.isEmpty() || line.startsWith('#') || line.startsWith("//")) {
            continue;
        }

        // 3. Tenta converter a linha de Hex (ex: 0xFB000313) para uint32_t
        bool ok;
        // O '0' no final autodetecta a base (ex: "0x" para hex, "10" para dec)
        uint32_t instrucao = line.toUInt(&ok, 0);

        if (!ok) {
            // Se falhar, avisa o usuário
            QMessageBox::critical(this, "Erro de Leitura",
                                  QString("Erro ao ler o arquivo na linha %1: \"%2\" nao e um numero hexadecimal valido.")
                                  .arg(lineNumber).arg(line));
            file.close();
            return;
        }

        // 4. Adiciona a instrução ao vetor
        programa.push_back(instrucao);
    }
    file.close();

    // 5. Se chegou aqui, a leitura foi um sucesso. Reseta o Core e carrega.

    // Adiciona uma instrução nula no final para garantir que o simulador pare,
    // caso o usuário esqueça.
    programa.push_back(0x00000000);

    m_runTimer->stop(); // Para a simulação se estiver rodando
    ui->runButton->setText("Run");
    ui->runButton->setEnabled(true);
    ui->stepButton->setEnabled(true);

    m_core->reset(); // Reseta o processador
    m_core->load_program(programa); // Carrega o NOVO programa

    ui->logView->clear(); // Limpa o log
    ui->logView->append(QString("Programa carregado de %1. Total de %2 instrucoes (+1 nula).")
                        .arg(filePath).arg(programa.size() - 1));
    updateUI(); // Atualiza a exibição dos registradores
}