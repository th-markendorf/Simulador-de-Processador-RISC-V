#include <QApplication>
#include "gui/launcherwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ESTILO VISUAL
    a.setStyleSheet(R"(
        /* --- 1. Janela e Fundos Gerais --- */
        QMainWindow, QWidget {
            background-color: #2b2b2b;
            color: #d4d4d4; /* Texto cinza claro, menos cansativo que branco puro */
            font-family: 'Segoe UI', 'Roboto', sans-serif;
            font-size: 10pt;
        }

        /* --- 2. Botões com Estilo Moderno --- */
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #007acc, stop:1 #005f9e);
            color: white;
            border: 1px solid #005f9e;
            padding: 8px 16px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #008be6, stop:1 #007acc);
            border: 1px solid #007acc;
        }
        QPushButton:pressed {
            background-color: #004080;
            border: 1px solid #003366;
            padding-top: 9px; /* Efeito de 'afundar' levemente */
            padding-bottom: 7px;
        }
        QPushButton:disabled {
            background-color: #3e3e3e;
            color: #777777;
            border: 1px solid #444444;
        }

        /* --- 3. Tabelas Sofisticadas (Registradores, Cache, RAM) --- */
        QTableWidget {
            background-color: #1e1e1e; /* Fundo mais escuro que a janela */
            gridline-color: #333333;
            color: #cccccc;
            border: 1px solid #3a3a3a;
            selection-background-color: #264f78; /* Azul escuro seleção */
            selection-color: white;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #333333;
            color: #e0e0e0;
            padding: 6px;
            border: none;
            border-right: 1px solid #444444;
            border-bottom: 1px solid #444444;
            font-weight: bold;
        }
        QTableCornerButton::section {
            background-color: #333333;
            border: none;
        }

        /* --- 4. Entradas de Dados (Campos de Texto e Spin) --- */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QComboBox {
            background-color: #3c3c3c;
            color: white;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 5px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid #007acc; /* Borda azul ao focar */
            background-color: #444444;
        }

        /* Estilizando o ComboBox (Dropdown) */
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #555555;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #cccccc; /* Triângulo CSS manual */
            margin-top: 2px;
            margin-right: 2px;
        }

        /* --- 5. Agrupamentos (Pipeline, Decodificação) --- */
        QGroupBox {
            border: 1px solid #505050;
            border-radius: 6px;
            margin-top: 24px;
            background-color: #2b2b2b;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 10px;
            color: #4fc1ff; /* Ciano claro */
            background-color: #2b2b2b;
            font-weight: bold;
        }

        /* --- 6. Abas (TabWidget) --- */
        QTabWidget::pane {
            border: 1px solid #444444;
            background-color: #2b2b2b;
            top: -1px; /* Conecta a aba ao painel */
        }
        QTabBar::tab {
            background: #3a3a3a;
            color: #aaaaaa;
            padding: 8px 25px;
            border: 1px solid #444444;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background: #2b2b2b;
            color: #4fc1ff;
            border-top: 2px solid #4fc1ff; /* Destaque no topo */
            border-bottom: 1px solid #2b2b2b; /* 'Abre' a aba para o conteúdo */
            font-weight: bold;
        }
        QTabBar::tab:hover {
            background: #444444;
            color: white;
        }

        /* --- 7. Barra de Menu --- */
        QMenuBar {
            background-color: #333333;
            color: #f0f0f0;
            border-bottom: 1px solid #444444;
        }
        QMenuBar::item:selected {
            background-color: #505050;
        }
        QMenu {
            background-color: #252526;
            color: white;
            border: 1px solid #454545;
        }
        QMenu::item {
            padding: 6px 30px 6px 20px;
        }
        QMenu::item:selected {
            background-color: #094771;
        }

        /* --- 8. Tooltips (Dicas flutuantes) --- */
        QToolTip {
            background-color: #111111;
            color: #4fc1ff;
            border: 1px solid #4fc1ff;
            padding: 4px;
        }

        /* --- 9. Barras de Rolagem (ScrollBars) - O "Toque Final" --- */
        QScrollBar:vertical {
            border: none;
            background: #2b2b2b;
            width: 12px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #555555;
            min-height: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background: #777777;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }

        /* Scrollbar Horizontal (para memória) */
        QScrollBar:horizontal {
            border: none;
            background: #2b2b2b;
            height: 12px;
        }
        QScrollBar::handle:horizontal {
            background: #555555;
            min-width: 20px;
            border-radius: 6px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #777777;
        }
    )");

    LauncherWindow w;
    w.show();
    return a.exec();
}