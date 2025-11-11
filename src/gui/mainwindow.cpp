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
#include <QTabWidget>
#include <QLineEdit>

static const std::array<QString, 32> abiNames = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// O construtor cria a janela e inicializa seu Core
MainWindow::MainWindow(Core *core, QWidget *parent)
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

    // Configura a tabela de memória (16 bytes por linha)
    const int memCols = 17; // 1 Endereço + 16 Bytes
    ui->memoryTable->setColumnCount(memCols + 1); // +1 para a coluna ASCII
    ui->memoryTable->setRowCount(16); // Vamos mostrar 16 linhas (16 * 16 = 256 bytes)

    // Configura os cabeçalhos das colunas
    QStringList memHeaders;
    memHeaders << "Endereço";
    for (int i = 0; i < 16; ++i) {
        memHeaders << QString("+%1").arg(i, 2, 16, QChar('0')).toUpper();
    }
    memHeaders << "ASCII";
    ui->memoryTable->setHorizontalHeaderLabels(memHeaders);

    // Trava a edição
    ui->memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Ajusta o tamanho das colunas
    ui->memoryTable->resizeColumnsToContents();

    m_core->reset(); // Garante que o core esteja zerado
    ui->logView->append("Simulador iniciado. Use 'Load' para carregar um programa.");
    updateUI(); // Atualiza a exibição (registradores zerados)

    // Desativa os botões de execução até que um programa seja carregado
    ui->runButton->setEnabled(false);
    ui->stepButton->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief Opção 1 (Carregar Programa)
 */
void MainWindow::on_loadButton_clicked() {
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
void MainWindow::on_resetButton_clicked() {
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
void MainWindow::on_stepButton_clicked() {
    // Chama a função step() do SEU Core
    m_core->tick_clock(); // A nova função!

    // tick_clock() é void, não retorna log.
    ui->logView->append("Clock tick executado.");

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
void MainWindow::on_runButton_clicked() {
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
void MainWindow::on_run_timer_timeout() {
    on_stepButton_clicked();
}

/**
 * @brief Opção 4 (Exibir Registradores)
 * Esta função é chamada automaticamente após cada ação.
 */
void MainWindow::updateUI() {
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
void MainWindow::loadProgramFromFile(const QString &filePath) {
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
                                  QString(
                                      "Erro ao ler o arquivo na linha %1: \"%2\" nao e um numero hexadecimal valido.")
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

void MainWindow::on_memInspectButton_clicked() {
    // 1. Obter o endereço do QLineEdit (memAddressInput)
    QString addressStr = ui->memAddressInput->text();
    bool ok;
    uint32_t startAddress = addressStr.toUInt(&ok, 0); // 0 auto-detecta base (Hex)

    if (!ok) {
        ui->logView->append("[ERRO] Endereço de memória inválido: " + addressStr);
        return;
    }

    // 2. Alinhar o endereço para 16 bytes (opcional, mas limpo)
    startAddress &= ~0x0F; // Zera os últimos 4 bits

    // 3. Preencher a tabela (16 linhas x 16 bytes)
    for (int row = 0; row < 16; ++row) {
        uint32_t rowAddress = startAddress + (row * 16);

        // Coluna 0: Endereço da Linha
        QString hexAddr = QString("0x%1").arg(rowAddress, 8, 16, QChar('0'));
        ui->memoryTable->setItem(row, 0, new QTableWidgetItem(hexAddr));

        QString asciiString = ""; // String para a última coluna

        // Colunas 1-16: Bytes de Dados
        for (int col = 0; col < 16; ++col) {
            uint32_t currentAddress = rowAddress + col;
            uint8_t byte = m_core->get_byte_memoria(currentAddress);

            // Coluna de Byte (Hex)
            QString hexByte = QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
            ui->memoryTable->setItem(row, col + 1, new QTableWidgetItem(hexByte));

            // Constrói a string ASCII
            if (byte >= 32 && byte <= 126) {
                // Caractere imprimível
                asciiString += static_cast<char>(byte);
            } else {
                asciiString += "."; // caractere não imprimível
            }
        }

        // Coluna 17: String ASCII
        ui->memoryTable->setItem(row, 17, new QTableWidgetItem(asciiString));
    }

    // 4. Ajustar colunas (apenas na primeira vez, opcional)
    ui->memoryTable->resizeColumnsToContents();
    ui->logView->append(
        QString("[INFO] Visualização da memória atualizada a partir de %1").arg(
            QString("0x%1").arg(startAddress, 8, 16, QChar('0'))));
}
