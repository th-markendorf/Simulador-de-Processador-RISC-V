#ifndef DEMOWINDOW_H
#define DEMOWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QHeaderView>

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

    void on_setRegButtonRs1_clicked();
    void on_setRegButtonRs2_clicked();

    // Slot para atualizar a UI quando a instrução mudar (ex: esconder rs2)
    void on_comboInstrucao_currentIndexChanged(int index);

    void on_resetRegsButton_clicked();

private:
    void updateRegistersView(QTableWidget *view, int highlightedRd);

    // Funções helper portadas do seu main.cpp
    uint32_t montar_tipo_R(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode);
    uint32_t montar_tipo_I(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode);

    Ui::DemoWindow *ui;
    Core *m_core; // Ponteiro para o Core (não o possui)

    const QBrush m_highlightBrush;
};

#endif // DEMOWINDOW_H
