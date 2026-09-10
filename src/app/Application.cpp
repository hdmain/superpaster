#include "app/Application.h"
#include "clipboard/ClipboardHistory.h"
#include "hotkeys/DesktopShortcut.h"
#include "hotkeys/GlobalHotkey.h"
#include "theme/ThemeManager.h"
#include "ui/OverlayWindow.h"
#include "ui/SettingsWindow.h"

#include <QApplication>
#include <QClipboard>
#include <QGuiApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

Application::Application(QObject* parent)
    : QObject(parent)
{
}

Application::~Application()
{
    if (m_desktopShortcut) {
        m_desktopShortcut->unregisterShortcut();
    }
    if (m_hotkey) {
        m_hotkey->unregisterHotkey();
    }
    delete m_overlay;
    m_overlay = nullptr;
    delete m_settings;
    m_settings = nullptr;
    delete m_trayMenu;
    m_trayMenu = nullptr;
}

bool Application::initialize(bool openSettings, bool openOverlay)
{
    m_themes = new ThemeManager(this);
    m_history = new ClipboardHistory(this);
    m_hotkey = new GlobalHotkey(this);
    m_desktopShortcut = new DesktopShortcut(this);

    m_themes->apply();

    m_settings = new SettingsWindow(m_themes, m_history);
    m_overlay = new OverlayWindow(m_history);

    connect(m_settings, &SettingsWindow::quitRequested, qApp, &QApplication::quit);
    connect(m_settings, &SettingsWindow::showOverlayRequested, this, &Application::toggleOverlay);
    connect(m_settings, &SettingsWindow::reregisterShortcutRequested, this, [this]() {
        if (m_desktopShortcut) {
            m_desktopShortcut->unregisterShortcut();
            m_desktopShortcut->registerShortcut();
            refreshShortcutStatus();
        }
    });
    connect(m_overlay, &OverlayWindow::itemChosen, this, &Application::pasteItem);
    connect(m_hotkey, &GlobalHotkey::activated, this, &Application::toggleOverlay);

    // Wayland (Pop!_OS / GNOME): register through desktop settings.
    // X11: also grab Super+V natively as a fallback.
    const bool desktopOk = m_desktopShortcut->registerShortcut();
    const bool x11Ok = m_hotkey->registerHotkey();
    if (!desktopOk && !x11Ok) {
        qWarning("Spaste: no global Super+V backend available — use Open overlay in settings");
    }
    refreshShortcutStatus();

    setupTray();

    if (openSettings) {
        showSettings();
    }
    if (openOverlay) {
        toggleOverlay();
    }

    return true;
}

void Application::toggleOverlay()
{
    if (m_overlay->isVisible()) {
        m_overlay->hideOverlay();
    } else {
        m_overlay->showOverlay();
    }
}

void Application::showSettings()
{
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();
}

void Application::pasteItem(const QString& text)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void Application::setupTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayMenu = new QMenu();
    m_trayMenu->addAction(tr("Settings"), this, &Application::showSettings);
    m_trayMenu->addAction(tr("Clipboard overlay"), this, &Application::toggleOverlay);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(tr("Quit"), qApp, &QApplication::quit);

    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/clipboard.svg")), this);
    m_tray->setToolTip(QStringLiteral("Spaste"));
    m_tray->setContextMenu(m_trayMenu);
    m_tray->show();

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            showSettings();
        }
    });
}

void Application::refreshShortcutStatus()
{
    QStringList parts;
    if (m_desktopShortcut && m_desktopShortcut->isRegistered()) {
        parts << m_desktopShortcut->statusText();
    } else if (m_desktopShortcut) {
        parts << m_desktopShortcut->statusText();
    }

    const QString platform = QGuiApplication::platformName();
    if (platform.contains(QStringLiteral("wayland"), Qt::CaseInsensitive)) {
        parts << tr("Session: Wayland (X11 key grab is inactive).");
    } else if (platform.contains(QStringLiteral("xcb"), Qt::CaseInsensitive)) {
        parts << tr("Session: X11.");
    }

    if (m_settings) {
        m_settings->setShortcutStatus(parts.join(QLatin1Char('\n')));
    }
}
