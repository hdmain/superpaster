#pragma once

#include <QWidget>
#include <QString>

class ThemeManager;
class ClipboardHistory;
class QComboBox;
class QLabel;

class SettingsWindow : public QWidget {
    Q_OBJECT

public:
    SettingsWindow(ThemeManager* themes, ClipboardHistory* history, QWidget* parent = nullptr);

    void setShortcutStatus(const QString& text);

signals:
    void quitRequested();
    void showOverlayRequested();
    void reregisterShortcutRequested();

private:
    void buildUi();

    ThemeManager* m_themes = nullptr;
    ClipboardHistory* m_history = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QLabel* m_shortcutStatus = nullptr;
};
