#pragma once
#include <QColor>
#include <QFont>
#include <QString>
#include <QApplication>
#include <QPalette>

// ============================================================
//  APP THEME — Deep Space Laboratory Aesthetic
//  Scientific dark theme with neon accents
// ============================================================
namespace Theme {

// Core palette
constexpr auto BG_BASE      = "#030810";  // near-black, deep blue
constexpr auto BG_PANEL     = "#0b1523";  // panel background
constexpr auto BG_WIDGET    = "#101d30";  // widget background
constexpr auto BG_HOVER     = "#162540";  // hover state
constexpr auto BORDER       = "#1c3d63";  // subtle borders
constexpr auto BORDER_BRIGHT = "#2a5a8a"; // bright borders

// Accent colors
constexpr auto CYAN         = "#00e5ff";  // success / active particles
constexpr auto CYAN_DIM     = "#007da8";  // dimmed cyan
constexpr auto GREEN        = "#39ff14";  // detector / arrival
constexpr auto GOLD         = "#ffd32a";  // RMS / warning
constexpr auto RED          = "#ff4060";  // lost / error
constexpr auto BLUE_FIELD   = "#4db8ff";  // field vectors
constexpr auto PURPLE       = "#b56bff";  // render / export

// Text
constexpr auto TEXT_BRIGHT  = "#e8f4ff";  // headings
constexpr auto TEXT_PRIMARY = "#c0d8f0";  // body
constexpr auto TEXT_DIM     = "#5070a0";  // secondary labels

// 3D tube
constexpr auto TUBE_SURFACE = "#0f2e54";
constexpr auto TUBE_LINES   = "#1a4a80";

inline QColor color(const char* hex)  { return QColor(hex); }
inline QColor color(const QString& h) { return QColor(h);   }

inline void apply(QApplication& app) {
    app.setStyle("Fusion");

    QPalette p;
    p.setColor(QPalette::Window,          color(BG_BASE));
    p.setColor(QPalette::WindowText,      color(TEXT_PRIMARY));
    p.setColor(QPalette::Base,            color(BG_PANEL));
    p.setColor(QPalette::AlternateBase,   color(BG_WIDGET));
    p.setColor(QPalette::ToolTipBase,     color(BG_WIDGET));
    p.setColor(QPalette::ToolTipText,     color(TEXT_PRIMARY));
    p.setColor(QPalette::Text,            color(TEXT_PRIMARY));
    p.setColor(QPalette::Button,          color(BG_WIDGET));
    p.setColor(QPalette::ButtonText,      color(TEXT_BRIGHT));
    p.setColor(QPalette::BrightText,      color(CYAN));
    p.setColor(QPalette::Link,            color(CYAN));
    p.setColor(QPalette::Highlight,       color(CYAN_DIM));
    p.setColor(QPalette::HighlightedText, color(TEXT_BRIGHT));
    p.setColor(QPalette::Mid,             color(BORDER));
    p.setColor(QPalette::Dark,            color(BG_BASE));
    p.setColor(QPalette::Light,           color(BG_HOVER));
    p.setColor(QPalette::Shadow,          color(BG_BASE));
    app.setPalette(p);

    app.setStyleSheet(R"(
        QWidget {
            background-color: #030810;
            color: #c0d8f0;
            font-family: 'SF Pro Text', 'Segoe UI', 'Ubuntu', sans-serif;
        }
        QMainWindow::separator { width: 2px; background: #1c3d63; }

        /* Panels & Frames */
        QFrame#panel {
            background: #0b1523;
            border: 1px solid #1c3d63;
            border-radius: 6px;
        }
        QGroupBox {
            background: #0b1523;
            border: 1px solid #1c3d63;
            border-radius: 6px;
            margin-top: 16px;
            padding: 8px;
            font-weight: 600;
            color: #5080a0;
            letter-spacing: 1px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 2px 8px;
            color: #00e5ff;
            background: transparent;
        }

        /* Buttons */
        QPushButton {
            background: #101d30;
            border: 1px solid #2a5a8a;
            border-radius: 4px;
            color: #c0d8f0;
            padding: 6px 16px;
            font-weight: 500;
            letter-spacing: 0.5px;
        }
        QPushButton:hover {
            background: #162540;
            border-color: #00e5ff;
            color: #e8f4ff;
        }
        QPushButton:pressed { background: #0d1a28; }
        QPushButton:disabled { opacity: 0.4; }
        QPushButton#btnPrimary {
            background: #005a6a;
            border-color: #00e5ff;
            color: #00e5ff;
            font-weight: 600;
        }
        QPushButton#btnPrimary:hover { background: #007a8f; }
        QPushButton#btnDanger {
            background: #3d1020;
            border-color: #ff4060;
            color: #ff4060;
        }
        QPushButton#btnDanger:hover { background: #5a1828; }
        QPushButton#btnGold {
            background: #2d2000;
            border-color: #ffd32a;
            color: #ffd32a;
        }
        QPushButton#btnGold:hover { background: #3d2d00; }
        QPushButton#btnPurple {
            background: #200840;
            border-color: #b56bff;
            color: #b56bff;
        }
        QPushButton#btnPurple:hover { background: #2d1058; }

        /* Slider */
        QSlider::groove:horizontal {
            height: 4px;
            background: #1c3d63;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #00e5ff;
            border: 2px solid #00e5ff;
            width: 14px; height: 14px;
            margin: -5px 0;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: #40f0ff;
            width: 16px; height: 16px;
            margin: -6px 0;
            border-radius: 8px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #004a5a, stop:1 #00e5ff);
            border-radius: 2px;
        }

        /* Labels */
        QLabel#labelTitle {
            color: #00e5ff;
            font-size: 18px;
            font-weight: 700;
            letter-spacing: 2px;
        }
        QLabel#labelSubtitle {
            color: #5070a0;
            font-size: 11px;
            letter-spacing: 1px;
        }
        QLabel#labelValue {
            color: #e8f4ff;
            font-family: 'JetBrains Mono', 'Fira Code', 'Courier New', monospace;
            font-size: 14px;
            font-weight: 600;
        }
        QLabel#labelKey {
            color: #5070a0;
            font-size: 11px;
        }

        /* Tables */
        QTableWidget {
            background: #0b1523;
            alternate-background-color: #101d30;
            gridline-color: #1c3d63;
            border: 1px solid #1c3d63;
            border-radius: 4px;
            selection-background-color: #007da8;
            font-family: 'JetBrains Mono', 'Courier New', monospace;
            font-size: 11px;
        }
        QTableWidget::item { padding: 4px 8px; }
        QHeaderView::section {
            background: #101d30;
            color: #5070a0;
            border: 0;
            border-bottom: 1px solid #1c3d63;
            padding: 6px 8px;
            font-size: 10px;
            font-weight: 600;
            letter-spacing: 1px;
            text-transform: uppercase;
        }

        /* Scroll Area */
        QScrollArea { border: none; background: transparent; }
        QScrollBar:vertical {
            background: #0b1523;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #2a5a8a;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #3a7ab5; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        /* Tab widget */
        QTabWidget::pane {
            border: 1px solid #1c3d63;
            border-radius: 4px;
            background: #0b1523;
        }
        QTabBar::tab {
            background: #101d30;
            color: #5070a0;
            border: 1px solid #1c3d63;
            padding: 6px 16px;
            border-bottom: none;
            border-radius: 4px 4px 0 0;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background: #0b1523;
            color: #00e5ff;
            border-color: #2a5a8a;
            border-bottom-color: #0b1523;
        }
        QTabBar::tab:hover:!selected {
            background: #162540;
            color: #c0d8f0;
        }

        /* Progress bar */
        QProgressBar {
            background: #101d30;
            border: 1px solid #1c3d63;
            border-radius: 3px;
            text-align: center;
            color: #c0d8f0;
            font-size: 11px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #004a5a, stop:1 #00e5ff);
            border-radius: 2px;
        }

        /* ComboBox */
        QComboBox {
            background: #101d30;
            border: 1px solid #2a5a8a;
            border-radius: 4px;
            padding: 4px 8px;
            color: #c0d8f0;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background: #0b1523;
            border: 1px solid #1c3d63;
            selection-background-color: #007da8;
        }

        /* SpinBox */
        QDoubleSpinBox, QSpinBox {
            background: #101d30;
            border: 1px solid #2a5a8a;
            border-radius: 4px;
            padding: 4px 8px;
            color: #c0d8f0;
        }

        /* Status bar */
        QStatusBar {
            background: #0b1523;
            border-top: 1px solid #1c3d63;
            color: #5070a0;
            font-size: 11px;
        }

        /* Toolbar */
        QToolBar {
            background: #0b1523;
            border-bottom: 1px solid #1c3d63;
            spacing: 4px;
            padding: 4px;
        }

        /* Splitter */
        QSplitter::handle {
            background: #1c3d63;
            width: 2px;
            height: 2px;
        }
        QSplitter::handle:hover { background: #00e5ff; }

        /* Tooltip */
        QToolTip {
            background: #0b1523;
            border: 1px solid #2a5a8a;
            color: #c0d8f0;
            padding: 4px 8px;
            border-radius: 4px;
        }

        /* CheckBox */
        QCheckBox { color: #c0d8f0; spacing: 8px; }
        QCheckBox::indicator {
            width: 14px; height: 14px;
            border: 1px solid #2a5a8a;
            border-radius: 3px;
            background: #101d30;
        }
        QCheckBox::indicator:checked {
            background: #007da8;
            border-color: #00e5ff;
            image: url(none);
        }
    )");
}

// Monospaced font for data display
inline QFont monoFont(int size = 11) {
    QFont f("JetBrains Mono");
    if (!f.exactMatch()) {
        f.setFamily("Fira Code");
        if (!f.exactMatch()) f.setFamily("Courier New");
    }
    f.setPointSize(size);
    return f;
}

// Label font
inline QFont labelFont(int size = 11, bool bold = false) {
    QFont f("SF Pro Display");
    if (!f.exactMatch()) f.setFamily("Segoe UI");
    f.setPointSize(size);
    f.setBold(bold);
    return f;
}

} // namespace Theme
