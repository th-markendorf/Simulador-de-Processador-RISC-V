#include "demowindow.h"
#include "ui_demowindow.h"
#include <sstream>
#include <iomanip>

#include <QTableWidget>
#include <QHeaderView>
#include <QBrush>

static const std::array<QString, 32> abiNames = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// Construtor
DemoWindow::DemoWindow(Core *core, QWidget *parent) : QMainWindow(parent),
                                                      ui(new Ui::DemoWindow),
                                                      m_core(core), // Armazena o ponteiro do Core
                                                      m_highlightBrush(QColor(80, 80, 80)) {
    ui->setupUi(this);
    ui->logView->setReadOnly(true);

    // Configura AMBAS as tabelas
    for (QTableWidget *table: {ui->registersTableAntes, ui->registersTableDepois}) {
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels(QStringList() << "Reg" << "ABI" << "Hex" << "Dec");
        table->setRowCount(33); // x0-x31 + pc

        // Adiciona os nomes dos registradores
        for (int i = 0; i < 32; ++i) {
            table->setItem(i, 0, new QTableWidgetItem(QString("x%1").arg(i)));
            table->setItem(i, 1, new QTableWidgetItem(abiNames[i]));
        }
        table->setItem(32, 0, new QTableWidgetItem("pc"));
        table->setItem(32, 1, new QTableWidgetItem("-"));

        // Trava a edição e o tamanho
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    }

    // Preenche o ComboBox com as instruções
    ui->comboInstrucao->addItem("ADD", 0x33);
    ui->comboInstrucao->addItem("SUB", 0x33);
    ui->comboInstrucao->addItem("ADDI", 0x13);
    ui->comboInstrucao->addItem("ANDI", 0x13);
    ui->comboInstrucao->addItem("ORI", 0x13);

    ui->comboInstrucao->addItem("MUL", 0x33);
    ui->comboInstrucao->addItem("MULH", 0x33);
    ui->comboInstrucao->addItem("MULHU", 0x33);
    ui->comboInstrucao->addItem("DIV", 0x33);
    ui->comboInstrucao->addItem("DIVU", 0x33);
    ui->comboInstrucao->addItem("REM", 0x33);
    ui->comboInstrucao->addItem("REMU", 0x33);

    // Configura os limites (substitui lerRegistrador/lerImediato)
    ui->spinRd->setRange(0, 31);
    ui->spinRs1->setRange(0, 31);
    ui->spinRs2->setRange(0, 31);
    ui->spinImm->setRange(-2048, 2047);

    // Inicia a UI
    on_comboInstrucao_currentIndexChanged(0);
    m_core->reset();
    updateRegistersView(ui->registersTableAntes, -1); // -1 = sem destaque
    updateRegistersView(ui->registersTableDepois, -1);
}

DemoWindow::~DemoWindow() {
    delete ui;
}

// Atualiza a UI baseado na instrução (ex: ADDI não usa rs2)
void DemoWindow::on_comboInstrucao_currentIndexChanged(int index) {
    uint32_t opcode = ui->comboInstrucao->itemData(index).toUInt();

    if (opcode == 0x33) {
        // 0x33 é o OPCODE_R (ADD, SUB, MUL, DIV, etc.)
        // Tipo R: Habilita rs2, desabilita imediato
        ui->spinRs2->setEnabled(true);
        ui->spinImm->setEnabled(false);
        ui->setRegButtonRs2->setEnabled(true);
    } else {
        // 0x13 é o OPCODE_I (ADDI, ANDI, etc.)
        // Tipo I: Desabilita rs2, habilita imediato
        ui->spinRs2->setEnabled(false);
        ui->spinImm->setEnabled(true);
        ui->setRegButtonRs2->setEnabled(false);
    }
}

/**
 * @brief Slot do botão "Definir Valor em RS1"
 */
void DemoWindow::on_setRegButtonRs1_clicked() {
    int32_t valor = ui->spinRegValor->value();
    int highlightReg1 = ui->spinRs1->value();

    // Define o valor apenas em rs1
    m_core->set_register(highlightReg1, valor);
    ui->logView->append(QString("Valor %1 definido em x%2 (rs1).").arg(valor).arg(highlightReg1));

    // Mostra o estado "Antes"
    updateRegistersView(ui->registersTableAntes, highlightReg1);
}

/**
 * @brief Slot do botão "Definir Valor em RS2"
 */
void DemoWindow::on_setRegButtonRs2_clicked() {
    int32_t valor = ui->spinRegValor->value();
    int highlightReg2 = ui->spinRs2->value();

    // Define o valor apenas em rs2
    m_core->set_register(highlightReg2, valor);
    ui->logView->append(QString("Valor %1 definido em x%2 (rs2).").arg(valor).arg(highlightReg2));

    // Mostra o estado "Antes"
    updateRegistersView(ui->registersTableAntes, highlightReg2);
}

void DemoWindow::on_execButton_clicked() {
    // 1. Pega os operandos da UI (Sem alterações)
    uint32_t rd = ui->spinRd->value();
    uint32_t rs1 = ui->spinRs1->value();
    uint32_t rs2 = ui->spinRs2->value();
    int32_t imm = ui->spinImm->value();
    QString instrucao = ui->comboInstrucao->currentText();

    uint32_t instrucao_codificada = 0;
    const uint32_t OPCODE_R = 0x33;
    const uint32_t OPCODE_I = 0x13;
    const uint32_t FUNCT7_M = 0x01;

    // 2. Monta a instrução (Sem alterações)
    if (instrucao == "ADD") {
        instrucao_codificada = montar_tipo_R(0x00, rs2, rs1, 0x0, rd, OPCODE_R);
    } else if (instrucao == "SUB") {
        instrucao_codificada = montar_tipo_R(0x20, rs2, rs1, 0x0, rd, OPCODE_R);
    } else if (instrucao == "MUL") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x0, rd, OPCODE_R);
    } else if (instrucao == "MULH") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x1, rd, OPCODE_R);
    } else if (instrucao == "MULHU") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x3, rd, OPCODE_R);
    } else if (instrucao == "DIV") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x4, rd, OPCODE_R);
    } else if (instrucao == "DIVU") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x5, rd, OPCODE_R);
    } else if (instrucao == "REM") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x6, rd, OPCODE_R);
    } else if (instrucao == "REMU") {
        instrucao_codificada = montar_tipo_R(FUNCT7_M, rs2, rs1, 0x7, rd, OPCODE_R);
    } else if (instrucao == "ADDI") {
        instrucao_codificada = montar_tipo_I(imm, rs1, 0x0, rd, OPCODE_I);
    } else if (instrucao == "ANDI") {
        instrucao_codificada = montar_tipo_I(imm, rs1, 0x7, rd, OPCODE_I);
    } else if (instrucao == "ORI") {
        instrucao_codificada = montar_tipo_I(imm, rs1, 0x6, rd, OPCODE_I);
    }

    // 3. Carrega e executa
    std::vector<uint32_t> programa_demo = {instrucao_codificada, 0x00000000};
    m_core->load_program(programa_demo);

    for (int i = 0; i < 5; ++i) {
        m_core->tick_clock();
    }

    updateRegistersView(ui->registersTableDepois, rd);
}
/**
 * @brief Função para exibir registradores em uma tabela, com destaque opcional.
 */
void DemoWindow::updateRegistersView(QTableWidget *view, int highlightedRd) {
    std::array<uint32_t, 32> regs = m_core->get_registradores();
    uint32_t pc = m_core->get_program_counter();

    // Reseta a cor de fundo padrão (necessário para limpar destaques antigos)
    const QBrush defaultBrush = view->palette().base();

    // 1. Formata e preenche os registradores (x0-x31)
    for (int i = 0; i < 32; ++i) { // O loop agora é 'i < 32'
        uint32_t valor = regs[i];

        // Coluna 2: Hexadecimal
        QString hexVal = QString("0x%1").arg(valor, 8, 16, QChar('0'));
        QTableWidgetItem* hexItem = new QTableWidgetItem(hexVal);

        // Coluna 3: Decimal
        QString decVal = QString::number(static_cast<int32_t>(valor));
        QTableWidgetItem* decItem = new QTableWidgetItem(decVal);

        // Define o destaque (ou reseta)
        if (i == highlightedRd && i != 0) { // Destaca se for o 'rd' (e não for x0)
            hexItem->setBackground(m_highlightBrush);
            decItem->setBackground(m_highlightBrush);
        } else {
            hexItem->setBackground(defaultBrush);
            decItem->setBackground(defaultBrush);
        }

        view->setItem(i, 2, hexItem);
        view->setItem(i, 3, decItem);
    }

    // 2. Formata e preenche o PC (Linha 32)
    QString hexPC = QString("0x%1").arg(pc, 8, 16, QChar('0'));
    QString decPC = QString::number(pc);

    QTableWidgetItem* pcHexItem = new QTableWidgetItem(hexPC);
    QTableWidgetItem* pcDecItem = new QTableWidgetItem(decPC);
    pcHexItem->setBackground(defaultBrush); // Garante que o PC não tenha destaque
    pcDecItem->setBackground(defaultBrush);

    view->setItem(32, 2, pcHexItem);
    view->setItem(32, 3, pcDecItem);
}

/**
 * @brief Slot para o botão "Resetar Registradores".
 * Reseta o core e limpa ambas as tabelas de visualização.
 */
void DemoWindow::on_resetRegsButton_clicked()
{
    // 1. Chama o reset do Core
    m_core->reset(); //

    // 2. Atualiza a tabela "Antes" (agora zerada)
    updateRegistersView(ui->registersTableAntes, -1); // -1 = sem destaque

    // 3. Atualiza a tabela "Depois" (agora zerada)
    updateRegistersView(ui->registersTableDepois, -1);

    // 4. Adiciona uma mensagem ao log
    ui->logView->append("Registradores resetados para 0.");
}

// Funções helper portadas do seu main.cpp
uint32_t DemoWindow::montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd,
                                   uint32_t opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t DemoWindow::montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (static_cast<uint32_t>(imm) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}
