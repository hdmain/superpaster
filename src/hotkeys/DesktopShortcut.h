#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// Registers Super+V through GNOME / Pop!_OS settings-daemon so it works on Wayland.
class DesktopShortcut : public QObject {
    Q_OBJECT

public:
    explicit DesktopShortcut(QObject* parent = nullptr);
    ~DesktopShortcut() override;

    [[nodiscard]] bool isSupported() const;
    [[nodiscard]] bool isRegistered() const;
    [[nodiscard]] QString statusText() const;

    bool registerShortcut();
    void unregisterShortcut();

private:
    [[nodiscard]] static QString gsettingsBin();
    [[nodiscard]] static QString runGsettings(const QStringList& args);
    [[nodiscard]] static bool gsettingsOk(const QStringList& args);
    void freeConflictingShellBindings();
    void restoreConflictingShellBindings();

    bool m_registered = false;
    bool m_freedMessageTray = false;
    QString m_previousMessageTray;
    QString m_status;
};
