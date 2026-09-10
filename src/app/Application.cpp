#include "app/Application.h"
#include "clipboard/ClipboardHistory.h"
#include "hotkeys/GlobalHotkey.h"
#include "theme/ThemeManager.h"
#include "ui/OverlayWindow.h"
#include "ui/SettingsWindow.h"

#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

Application::Application(QObject* parent)
    : QObject(parent)
{
}

Application::~Application()
{
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

bool Application::initialize()
{
    m_themes = new ThemeManager(this);
    m_history = new ClipboardHistory(this);
    m_hotkey = new GlobalHotkey(this);

    m_themes->apply();

    m_settings = new SettingsWindow(m_themes, m_history);
    m_overlay = new OverlayWindow(m_history);

    connect(m_settings, &SettingsWindow::quitRequested, qApp, &QApplication::quit);
    connect(m_settings, &SettingsWindow::showOverlayRequested, this, &Application::toggleOverlay);
    connect(m_overlay, &OverlayWindow::itemChosen, this, &Application::pasteItem);
    connect(m_hotkey, &GlobalHotkey::activated, this, &Application::toggleOverlay);

    if (!m_hotkey->registerHotkey()) {
        // Settings still useful; overlay can be opened from the settings window.
    }

    setupTray();

    // Launching Spaste opens settings by default
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();

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
    m_trayMenu->addAction(tr("Settings"), m_settings, &QWidget::show);
    m_trayMenu->addAction(tr("Clipboard overlay"), this, &Application::toggleOverlay);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(tr("Quit"), qApp, &QApplication::quit);

    m_tray = new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/clipboard.svg")), this);
    m_tray->setToolTip(QStringLiteral("Spaste"));
    m_tray->setContextMenu(m_trayMenu);
    m_tray->show();

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            m_settings->show();
            m_settings->raise();
            m_settings->activateWindow();
        }
    });
}
