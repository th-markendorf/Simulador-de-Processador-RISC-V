#include "demowindow.h"
#include "ui_demowindow.h"
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <sstream>
#include <iomanip>

static const std::array<QString, 32> abiNames = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// Construtor
DemoWindow::DemoWindow(Core *core, QWidget *parent) : QMainWindow(parent),
                                                      ui(new Ui::DemoWindow),
                                                      m_core(core),
                                                      m_highlightBrush(QColor(80, 80, 80)) {
    ui->setupUi(this);

    // CORREÇÃO DE LAYOUT

    ui->label_9->setGeometry(240, 30, 130, 20);
    ui->spinRegValor->setGeometry(240, 55, 120, 28);

    int btnWidth = 160;
    int btnHeight = 28;
    int col1_X = 400;
    int col2_X = 580;
    int row1_Y = 30;
    int row2_Y = 65;

    // 3. Aplica as posições
    ui->setRegButtonRs1->setGeometry(col1_X, row1_Y, btnWidth, btnHeight);
    ui->setRegButtonRs2->setGeometry(col1_X, row2_Y, btnWidth, btnHeight);
    ui->resetRegsButton->setGeometry(col2_X, row1_Y, btnWidth, btnHeight);
    ui->execButton->setGeometry(col2_X, row2_Y, btnWidth, btnHeight);

    ui->resetRegsButton->setText("Resetar Tudo");

    QString estiloCompacto = "font-size: 9pt; padding: 2px;";
    ui->setRegButtonRs1->setStyleSheet(estiloCompacto);
    ui->setRegButtonRs2->setStyleSheet(estiloCompacto);
    ui->resetRegsButton->setStyleSheet(estiloCompacto);
    ui->execButton->setStyleSheet(estiloCompacto);


    // MENU DE AJUDA
    QMenu *menuAjuda = ui->menubar->addMenu("Ajuda");
    QAction *actionInstrucoes = new QAction("Explicação das Instruções", this);

    connect(actionInstrucoes, &QAction::triggered, this, [this]() {
        QString texto =
            "<h3>Guia de Instruções - Modo Demo</h3>"
            "<p>Neste modo, você testa a execução isolada na ULA (Unidade Lógica e Aritmética).</p>"
            "<hr>"
            "<h4>Instruções Tipo-R (Usa RS1 e RS2)</h4>"
            "<ul>"
            "<li><b>ADD:</b> Soma (<i>rd = rs1 + rs2</i>)</li>"
            "<li><b>SUB:</b> Subtração (<i>rd = rs1 - rs2</i>)</li>"
            "<li><b>MUL:</b> Multiplicação (<i>rd = rs1 * rs2</i>)</li>"
            "<li><b>MULH/MULHU:</b> Multiplicação (parte alta do resultado)</li>"
            "<li><b>DIV/DIVU:</b> Divisão Inteira (com/sem sinal)</li>"
            "<li><b>REM/REMU:</b> Resto da Divisão (com/sem sinal)</li>"
            "</ul>"
            "<hr>"
            "<h4>Instruções Tipo-I (Usa RS1 e Imediato)</h4>"
            "<ul>"
            "<li><b>ADDI:</b> Soma com Constante (<i>rd = rs1 + imediato</i>)</li>"
            "<li><b>ANDI:</b> 'E' Lógico bit-a-bit (<i>rd = rs1 & imediato</i>)</li>"
            "<li><b>ORI:</b> 'OU' Lógico bit-a-bit (<i>rd = rs1 | imediato</i>)</li>"
            "<li><b>JALR:</b> Pulo Indireto (<i>PC = rs1 + imediato</i>, salva PC+4 em rd)</li>"
            "</ul>"
            "<hr>"
            "<h4>Instruções Tipo-U (Usa Apenas Imediato)</h4>"
            "<ul>"
            "<li><b>AUIPC:</b> Soma PC (<i>rd = PC + (imediato << 12)</i>)</li>"
            "</ul>";

        QMessageBox::about(this, "Instruções Suportadas", texto);
    });

    menuAjuda->addAction(actionInstrucoes);

    ui->logView->setReadOnly(true);

    for (QTableWidget *table: {ui->registersTableAntes, ui->registersTableDepois}) {
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels(QStringList() << "Reg" << "ABI" << "Hex" << "Dec");
        table->setRowCount(33);

        for (int i = 0; i < 32; ++i) {
            table->setItem(i, 0, new QTableWidgetItem(QString("x%1").arg(i)));
            table->setItem(i, 1, new QTableWidgetItem(abiNames[i]));
        }
        table->setItem(32, 0, new QTableWidgetItem("pc"));
        table->setItem(32, 1, new QTableWidgetItem("-"));

        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    }

    // ADICIONANDO AS NOVAS INSTRUÇÕES AQUI
    ui->comboInstrucao->addItem("ADD", 0x33);
    ui->comboInstrucao->addItem("SUB", 0x33);
    ui->comboInstrucao->addItem("MUL", 0x33);
    ui->comboInstrucao->addItem("MULH", 0x33);
    ui->comboInstrucao->addItem("MULHU", 0x33);
    ui->comboInstrucao->addItem("DIV", 0x33);
    ui->comboInstrucao->addItem("DIVU", 0x33);
    ui->comboInstrucao->addItem("REM", 0x33);
    ui->comboInstrucao->addItem("REMU", 0x33);

    ui->comboInstrucao->addItem("ADDI", 0x13);
    ui->comboInstrucao->addItem("ANDI", 0x13);
    ui->comboInstrucao->addItem("ORI", 0x13);

    // Novas:
    ui->comboInstrucao->addItem("JALR", 0x67);  // Tipo I
    ui->comboInstrucao->addItem("AUIPC", 0x17); // Tipo U

    ui->spinRd->setRange(0, 31);
    ui->spinRs1->setRange(0, 31);
    ui->spinRs2->setRange(0, 31);
    ui->spinImm->setRange(-2048, 2047);

    on_comboInstrucao_currentIndexChanged(0);
    m_core->reset();
    updateRegistersView(ui->registersTableAntes, -1);
    updateRegistersView(ui->registersTableDepois, -1);
}

DemoWindow::~DemoWindow() {
    delete ui;
}

// Atualiza a UI baseado na instrução (Habilita/Desabilita campos)
void DemoWindow::on_comboInstrucao_currentIndexChanged(int index) {
    uint32_t opcode = ui->comboInstrucao->itemData(index).toUInt();

    if (opcode == 0x33) {
        // Tipo R (ADD, SUB...): Usa RS1, RS2. Sem Imediato.
        ui->spinRs1->setEnabled(true);
        ui->setRegButtonRs1->setEnabled(true);

        ui->spinRs2->setEnabled(true);
        ui->setRegButtonRs2->setEnabled(true);

        ui->spinImm->setEnabled(false);

    } else if (opcode == 0x17) {
        // Tipo U (AUIPC): Usa Imediato. Sem RS1, Sem RS2.
        ui->spinRs1->setEnabled(false); // AUIPC não lê registrador
        ui->setRegButtonRs1->setEnabled(false);

        ui->spinRs2->setEnabled(false);
        ui->setRegButtonRs2->setEnabled(false);

        ui->spinImm->setEnabled(true);

    } else {
        // Tipo I (ADDI, JALR...): Usa RS1, Imediato. Sem RS2.
        ui->spinRs1->setEnabled(true);
        ui->setRegButtonRs1->setEnabled(true);

        ui->spinRs2->setEnabled(false);
        ui->setRegButtonRs2->setEnabled(false);

        ui->spinImm->setEnabled(true);
    }
}

void DemoWindow::on_setRegButtonRs1_clicked() {
    int32_t valor = ui->spinRegValor->value();
    int highlightReg1 = ui->spinRs1->value();
    m_core->set_register(highlightReg1, valor);
    ui->logView->append(QString("Valor %1 definido em x%2 (rs1).").arg(valor).arg(highlightReg1));
    updateRegistersView(ui->registersTableAntes, highlightReg1);
}

void DemoWindow::on_setRegButtonRs2_clicked() {
    int32_t valor = ui->spinRegValor->value();
    int highlightReg2 = ui->spinRs2->value();
    m_core->set_register(highlightReg2, valor);
    ui->logView->append(QString("Valor %1 definido em x%2 (rs2).").arg(valor).arg(highlightReg2));
    updateRegistersView(ui->registersTableAntes, highlightReg2);
}

void DemoWindow::on_execButton_clicked() {
    // 1. Pega os operandos da UI
    uint32_t rd = ui->spinRd->value();
    uint32_t rs1 = ui->spinRs1->value();
    uint32_t rs2 = ui->spinRs2->value();
    int32_t imm = ui->spinImm->value();
    QString instrucao = ui->comboInstrucao->currentText();

    uint32_t instrucao_codificada = 0;
    const uint32_t OPCODE_R = 0x33;
    const uint32_t OPCODE_I = 0x13;
    const uint32_t FUNCT7_M = 0x01;

    // 2. Monta a instrução
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

    } else if (instrucao == "JALR") {
        // JALR é Tipo-I, opcode 0x67, funct3 0x0
        instrucao_codificada = montar_tipo_I(imm, rs1, 0x0, rd, 0x67);

    } else if (instrucao == "AUIPC") {
        // AUIPC é Tipo-U (Imediato << 12 | rd << 7 | opcode)
        // O valor do spinImm (ex: 1) será deslocado para virar 4096 (0x1000)
        uint32_t u_imm = static_cast<uint32_t>(imm) & 0xFFFFF; // 20 bits
        instrucao_codificada = (u_imm << 12) | (rd << 7) | 0x17;
    }

    // Carrega e executa
    // Adiciona NOPs depois para garantir que o pipeline flua
    std::vector<uint32_t> programa_demo = {instrucao_codificada, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    m_core->load_program(programa_demo);

    // Executa ciclos suficientes para a instrução passar pelo WB
    for (int i = 0; i < 6; ++i) {
        m_core->tick_clock();
    }

    updateRegistersView(ui->registersTableDepois, rd);
}

void DemoWindow::updateRegistersView(QTableWidget *view, int highlightedRd) {
    std::array<uint32_t, 32> regs = m_core->get_registradores();
    uint32_t pc = m_core->get_program_counter();
    const QBrush defaultBrush = view->palette().base();

    for (int i = 0; i < 32; ++i) {
        uint32_t valor = regs[i];
        QString hexVal = QString("0x%1").arg(valor, 8, 16, QChar('0'));
        QTableWidgetItem* hexItem = new QTableWidgetItem(hexVal);
        QString decVal = QString::number(static_cast<int32_t>(valor));
        QTableWidgetItem* decItem = new QTableWidgetItem(decVal);

        if (i == highlightedRd && i != 0) {
            hexItem->setBackground(m_highlightBrush);
            decItem->setBackground(m_highlightBrush);
        } else {
            hexItem->setBackground(defaultBrush);
            decItem->setBackground(defaultBrush);
        }

        view->setItem(i, 2, hexItem);
        view->setItem(i, 3, decItem);
    }

    QString hexPC = QString("0x%1").arg(pc, 8, 16, QChar('0'));
    QString decPC = QString::number(pc);
    QTableWidgetItem* pcHexItem = new QTableWidgetItem(hexPC);
    QTableWidgetItem* pcDecItem = new QTableWidgetItem(decPC);
    pcHexItem->setBackground(defaultBrush);
    pcDecItem->setBackground(defaultBrush);

    view->setItem(32, 2, pcHexItem);
    view->setItem(32, 3, pcDecItem);
}

void DemoWindow::on_resetRegsButton_clicked()
{
    m_core->reset();
    updateRegistersView(ui->registersTableAntes, -1);
    updateRegistersView(ui->registersTableDepois, -1);
    ui->logView->append("Registradores resetados para 0.");
}

uint32_t DemoWindow::montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd,
                                   uint32_t opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t DemoWindow::montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (static_cast<uint32_t>(imm) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}