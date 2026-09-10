#pragma once

#include <QObject>

class ThemeManager;
class ClipboardHistory;
class GlobalHotkey;
class DesktopShortcut;
class SettingsWindow;
class OverlayWindow;
class QSystemTrayIcon;
class QMenu;
class SingleInstance;

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(QObject* parent = nullptr);
    ~Application() override;

    bool initialize(bool openSettings, bool openOverlay);

public slots:
    void toggleOverlay();
    void showSettings();

private:
    void pasteItem(const QString& text);
    void setupTray();
    void refreshShortcutStatus();

    ThemeManager* m_themes = nullptr;
    ClipboardHistory* m_history = nullptr;
    GlobalHotkey* m_hotkey = nullptr;
    DesktopShortcut* m_desktopShortcut = nullptr;
    SettingsWindow* m_settings = nullptr;
    OverlayWindow* m_overlay = nullptr;
    QSystemTrayIcon* m_tray = nullptr;
    QMenu* m_trayMenu = nullptr;
};
