#pragma once

#include <QWidget>

class ThemeManager;
class ClipboardHistory;
class QComboBox;

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(ThemeManager* themes, ClipboardHistory* history, QWidget* parent = nullptr);

signals:
    void quitRequested();
    void showOverlayRequested();

private:
    void buildUi();

    ThemeManager* m_themes = nullptr;
    ClipboardHistory* m_history = nullptr;
    QComboBox* m_themeCombo = nullptr;
};
