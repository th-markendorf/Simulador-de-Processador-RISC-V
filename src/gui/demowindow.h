#ifndef DEMOWINDOW_H
#define DEMOWINDOW_H

#include <QMainWindow>
#include <QTextEdit>

#include "core/Core.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class DemoWindow;
}

QT_END_NAMESPACE

class DemoWindow : public QMainWindow {
    Q_OBJECT

public:
    // Ela recebe o ponteiro do Core, mas não o possui
    explicit DemoWindow(Core *core, QWidget *parent = nullptr);

    ~DemoWindow();

private slots:
    // Slot para o botão "Executar"
    void on_execButton_clicked();

    // Slot para o botão "Definir Valor"
    void on_setRegButton_clicked();

    // Slot para atualizar a UI quando a instrução mudar (ex: esconder rs2)
    void on_comboInstrucao_currentIndexChanged(int index);

private:
    void updateRegistersView(QTextEdit *view); // Mostra os registradores

    // Funções helper portadas do seu main.cpp
    uint32_t montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode);

    uint32_t montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode);

    Ui::DemoWindow *ui;
    Core *m_core; // Ponteiro para o Core (não o possui)
};

#endif // DEMOWINDOW_H
