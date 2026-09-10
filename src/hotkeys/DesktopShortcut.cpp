#include "hotkeys/DesktopShortcut.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

namespace {
constexpr auto kBindingPath =
    "/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/spaste-overlay/";
constexpr auto kBindingSchema =
    "org.gnome.settings-daemon.plugins.media-keys.custom-keybinding";
constexpr auto kMediaKeysSchema =
    "org.gnome.settings-daemon.plugins.media-keys";
constexpr auto kShellKeybindingsSchema =
    "org.gnome.shell.keybindings";
} // namespace

DesktopShortcut::DesktopShortcut(QObject* parent)
    : QObject(parent)
{
    if (!isSupported()) {
        m_status = tr("Desktop shortcut API not available (non-GNOME session).");
    }
}

DesktopShortcut::~DesktopShortcut()
{
    unregisterShortcut();
}

bool DesktopShortcut::isSupported() const
{
    if (gsettingsBin().isEmpty()) {
        return false;
    }

    // Probe the media-keys schema used by GNOME and Pop!_OS.
    return gsettingsOk({QStringLiteral("list-keys"), QString::fromUtf8(kMediaKeysSchema)});
}

bool DesktopShortcut::isRegistered() const
{
    return m_registered;
}

QString DesktopShortcut::statusText() const
{
    return m_status;
}

bool DesktopShortcut::registerShortcut()
{
    if (!isSupported()) {
        m_status = tr("Could not register Super+V - gsettings/GNOME keybindings unavailable.");
        return false;
    }

    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath();
    if (appPath.isEmpty() || !QFileInfo::exists(appPath)) {
        m_status = tr("Could not resolve Spaste binary path for the shortcut.");
        return false;
    }

    freeConflictingShellBindings();

    // Ensure our custom keybinding path is listed.
    const QString existing = runGsettings(
        {QStringLiteral("get"), QString::fromUtf8(kMediaKeysSchema), QStringLiteral("custom-keybindings")});
    QStringList paths;
    // existing looks like: ['/path/one/', '/path/two/'] or @as []
    const QString trimmed = existing.trimmed();
    if (trimmed.contains(QLatin1Char('/'))) {
        const QString inner = trimmed.mid(trimmed.indexOf(QLatin1Char('[')) + 1,
                                          trimmed.lastIndexOf(QLatin1Char(']')) - trimmed.indexOf(QLatin1Char('[')) - 1);
        for (QString part : inner.split(QLatin1Char(','))) {
            part = part.trimmed();
            if (part.startsWith(QLatin1Char('\'')) && part.endsWith(QLatin1Char('\''))) {
                part = part.mid(1, part.size() - 2);
            }
            if (!part.isEmpty()) {
                paths.push_back(part);
            }
        }
    }
    if (!paths.contains(QString::fromUtf8(kBindingPath))) {
        paths.push_back(QString::fromUtf8(kBindingPath));
    }

    QString arrayValue = QStringLiteral("[");
    for (int i = 0; i < paths.size(); ++i) {
        if (i > 0) {
            arrayValue += QLatin1Char(',');
        }
        arrayValue += QLatin1Char('\'') + paths[i] + QLatin1Char('\'');
    }
    arrayValue += QLatin1Char(']');

    if (!gsettingsOk({QStringLiteral("set"), QString::fromUtf8(kMediaKeysSchema),
                      QStringLiteral("custom-keybindings"), arrayValue})) {
        m_status = tr("Failed to register custom keybinding list.");
        restoreConflictingShellBindings();
        return false;
    }

    const QString reloc = QStringLiteral("%1:%2")
                              .arg(QString::fromUtf8(kBindingSchema), QString::fromUtf8(kBindingPath));
    // gnome-settings-daemon parses this with g_shell_parse_argv.
    const QString command = appPath.contains(QLatin1Char(' '))
                                ? QStringLiteral("\"%1\" --toggle-overlay").arg(appPath)
                                : QStringLiteral("%1 --toggle-overlay").arg(appPath);

    const bool ok = gsettingsOk({QStringLiteral("set"), reloc, QStringLiteral("name"), QStringLiteral("Spaste Overlay")})
        && gsettingsOk({QStringLiteral("set"), reloc, QStringLiteral("command"), command})
        && gsettingsOk({QStringLiteral("set"), reloc, QStringLiteral("binding"), QStringLiteral("<Super>v")});

    if (!ok) {
        m_status = tr("Failed to write Super+V custom keybinding.");
        restoreConflictingShellBindings();
        return false;
    }

    m_registered = true;
    m_status = tr("Super+V registered via desktop settings (works on Wayland / Pop!_OS).");
    qInfo() << "Spaste:" << m_status;
    return true;
}

void DesktopShortcut::unregisterShortcut()
{
    if (!m_registered && !m_freedMessageTray) {
        return;
    }

    if (isSupported()) {
        const QString existing = runGsettings(
            {QStringLiteral("get"), QString::fromUtf8(kMediaKeysSchema), QStringLiteral("custom-keybindings")});
        QStringList paths;
        const QString trimmed = existing.trimmed();
        if (trimmed.contains(QLatin1Char('/'))) {
            const QString inner = trimmed.mid(trimmed.indexOf(QLatin1Char('[')) + 1,
                                              trimmed.lastIndexOf(QLatin1Char(']')) - trimmed.indexOf(QLatin1Char('[')) - 1);
            for (QString part : inner.split(QLatin1Char(','))) {
                part = part.trimmed();
                if (part.startsWith(QLatin1Char('\'')) && part.endsWith(QLatin1Char('\''))) {
                    part = part.mid(1, part.size() - 2);
                }
                if (!part.isEmpty() && part != QLatin1String(kBindingPath)) {
                    paths.push_back(part);
                }
            }
        }

        QString arrayValue = QStringLiteral("[");
        for (int i = 0; i < paths.size(); ++i) {
            if (i > 0) {
                arrayValue += QLatin1Char(',');
            }
            arrayValue += QLatin1Char('\'') + paths[i] + QLatin1Char('\'');
        }
        arrayValue += QLatin1Char(']');
        gsettingsOk({QStringLiteral("set"), QString::fromUtf8(kMediaKeysSchema),
                     QStringLiteral("custom-keybindings"), arrayValue});

        const QString reloc = QStringLiteral("%1:%2")
                                  .arg(QString::fromUtf8(kBindingSchema), QString::fromUtf8(kBindingPath));
        gsettingsOk({QStringLiteral("reset"), reloc, QStringLiteral("name")});
        gsettingsOk({QStringLiteral("reset"), reloc, QStringLiteral("command")});
        gsettingsOk({QStringLiteral("reset"), reloc, QStringLiteral("binding")});
    }

    restoreConflictingShellBindings();
    m_registered = false;
}

QString DesktopShortcut::gsettingsBin()
{
    return QStandardPaths::findExecutable(QStringLiteral("gsettings"));
}

QString DesktopShortcut::runGsettings(const QStringList& args)
{
    QProcess proc;
    proc.start(gsettingsBin(), args);
    if (!proc.waitForFinished(3000)) {
        proc.kill();
        return {};
    }
    return QString::fromUtf8(proc.readAllStandardOutput());
}

bool DesktopShortcut::gsettingsOk(const QStringList& args)
{
    QProcess proc;
    proc.start(gsettingsBin(), args);
    if (!proc.waitForFinished(3000)) {
        proc.kill();
        return false;
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        const QString err = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        if (!err.isEmpty()) {
            qWarning() << "gsettings" << args << "->" << err;
        }
        return false;
    }
    return true;
}

void DesktopShortcut::freeConflictingShellBindings()
{
    // GNOME / Pop!_OS bind Super+V to the notification tray by default.
    m_previousMessageTray = runGsettings(
                                 {QStringLiteral("get"), QString::fromUtf8(kShellKeybindingsSchema),
                                  QStringLiteral("toggle-message-tray")})
                                 .trimmed();
    if (m_previousMessageTray.contains(QStringLiteral("<Super>v"), Qt::CaseInsensitive)
        || m_previousMessageTray.contains(QStringLiteral("<Super>V"))) {
        if (gsettingsOk({QStringLiteral("set"), QString::fromUtf8(kShellKeybindingsSchema),
                         QStringLiteral("toggle-message-tray"), QStringLiteral("@as []")})) {
            m_freedMessageTray = true;
            qInfo() << "Spaste: freed Super+V from GNOME toggle-message-tray";
        }
    }
}

void DesktopShortcut::restoreConflictingShellBindings()
{
    if (!m_freedMessageTray) {
        return;
    }
    if (!m_previousMessageTray.isEmpty()) {
        gsettingsOk({QStringLiteral("set"), QString::fromUtf8(kShellKeybindingsSchema),
                     QStringLiteral("toggle-message-tray"), m_previousMessageTray});
    }
    m_freedMessageTray = false;
}
