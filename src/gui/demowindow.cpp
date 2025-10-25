#include "demowindow.h"
#include <sstream>
#include <iomanip>
#include "ui_demowindow.h"

// Construtor
DemoWindow::DemoWindow(Core *core, QWidget *parent) : QMainWindow(parent),
                                                      ui(new Ui::DemoWindow),
                                                      m_core(core) // Armazena o ponteiro do Core
{
    ui->setupUi(this);
    ui->logView->setReadOnly(true);
    ui->registersViewAntes->setReadOnly(true);
    ui->registersViewDepois->setReadOnly(true);

    // Preenche o ComboBox com as instruções
    ui->comboInstrucao->addItem("ADD", 0x33);
    ui->comboInstrucao->addItem("SUB", 0x33);
    ui->comboInstrucao->addItem("ADDI", 0x13);
    ui->comboInstrucao->addItem("ANDI", 0x13);
    ui->comboInstrucao->addItem("ORI", 0x13);

    // Configura os limites (substitui lerRegistrador/lerImediato)
    ui->spinRd->setRange(0, 31);
    ui->spinRs1->setRange(0, 31);
    ui->spinRs2->setRange(0, 31);
    ui->spinImm->setRange(-2048, 2047);

    // Inicia a UI
    on_comboInstrucao_currentIndexChanged(0);
    m_core->reset();
    updateRegistersView(ui->registersViewAntes);
    updateRegistersView(ui->registersViewDepois);
}

DemoWindow::~DemoWindow() {
    delete ui;
}

// Atualiza a UI baseado na instrução (ex: ADDI não usa rs2)
void DemoWindow::on_comboInstrucao_currentIndexChanged(int index) {
    QString instrucao = ui->comboInstrucao->currentText();

    if (instrucao == "ADD" || instrucao == "SUB") {
        // Tipo R: Habilita rs2, desabilita imediato
        ui->spinRs2->setEnabled(true);
        ui->spinImm->setEnabled(false);
    } else {
        // Tipo I: Desabilita rs2, habilita imediato
        ui->spinRs2->setEnabled(false);
        ui->spinImm->setEnabled(true);
    }
}

// Slot do botão "Definir Valor (em rs1/rs2)"
void DemoWindow::on_setRegButton_clicked() {
    int32_t valor = ui->spinRegValor->value();

    // Define o valor em rs1
    m_core->set_register(ui->spinRs1->value(), valor);

    // Se for Tipo R, define também em rs2
    if (ui->spinRs2->isEnabled()) {
        m_core->set_register(ui->spinRs2->value(), valor);
        ui->logView->append(QString("Valores definidos em rs1 e rs2."));
    } else {
        ui->logView->append(QString("Valor definido em rs1."));
    }

    // Mostra o estado "Antes"
    updateRegistersView(ui->registersViewAntes);
}

// Slot do botão "Executar Instrução"
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

    // 2. Monta a instrução (lógica do seu main.cpp)
    if (instrucao == "ADD") {
        instrucao_codificada = montar_tipo_R(0x00, rs2, rs1, 0x0, rd, OPCODE_R);
    } else if (instrucao == "SUB") {
        instrucao_codificada = montar_tipo_R(0x20, rs2, rs1, 0x0, rd, OPCODE_R);
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

    std::string log = m_core->step();
    ui->logView->append(QString::fromStdString(log));

    // 4. Mostra o resultado
    updateRegistersView(ui->registersViewDepois);
}

// Função para exibir registradores (como a updateUI da MainWindow)
void DemoWindow::updateRegistersView(QTextEdit *view) {
    std::array<uint32_t, 32> regs = m_core->get_registradores();
    uint32_t pc = m_core->get_program_counter();

    std::stringstream ss_regs;
    ss_regs << std::hex << std::setfill('0');

    for (int i = 0; i < 32; ++i) {
        ss_regs << "x" << std::setw(2) << std::dec << i << ": "
                << "0x" << std::setw(8) << std::hex << regs[i] << "\n";
    }
    ss_regs << "pc: " << "0x" << std::setw(8) << std::hex << pc << "\n";

    view->setText(QString::fromStdString(ss_regs.str()));
}

// Funções helper portadas do seu main.cpp
uint32_t DemoWindow::montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd,
                                   uint32_t opcode) {
    return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t DemoWindow::montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
    return (static_cast<uint32_t>(imm) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}
