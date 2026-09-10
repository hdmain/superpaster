#pragma once

#include <QObject>

class QApplication;
class ThemeManager;
class ClipboardHistory;
class GlobalHotkey;
class SettingsWindow;
class OverlayWindow;
class QSystemTrayIcon;
class QMenu;

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(QObject* parent = nullptr);
    ~Application() override;

    bool initialize();

private:
    void toggleOverlay();
    void pasteItem(const QString& text);
    void setupTray();

    ThemeManager* m_themes = nullptr;
    ClipboardHistory* m_history = nullptr;
    GlobalHotkey* m_hotkey = nullptr;
    SettingsWindow* m_settings = nullptr;
    OverlayWindow* m_overlay = nullptr;
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_trayMenu = nullptr;
};
