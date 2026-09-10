#include "app/Application.h"
#include "app/SingleInstance.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication::setApplicationName(QStringLiteral("Spaste"));
    QApplication::setOrganizationName(QStringLiteral("Spaste"));
    QApplication::setApplicationVersion(QStringLiteral(SPASTE_VERSION));
    QApplication::setDesktopFileName(QStringLiteral("spaste"));

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Spaste - RAM-only clipboard history"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption toggleOption(QStringList{QStringLiteral("toggle-overlay"), QStringLiteral("o")},
                                    QStringLiteral("Toggle the clipboard overlay (or start it)"));
    QCommandLineOption settingsOption(QStringList{QStringLiteral("settings"), QStringLiteral("s")},
                                      QStringLiteral("Show the settings window"));
    parser.addOption(toggleOption);
    parser.addOption(settingsOption);
    parser.process(app);

    const bool wantOverlay = parser.isSet(toggleOption);
    const bool wantSettings = parser.isSet(settingsOption) || (!wantOverlay);

    SingleInstance instance;
    if (!instance.tryBecomePrimary()) {
        if (wantOverlay) {
            instance.sendToPrimary(QByteArrayLiteral("toggle-overlay"));
        } else {
            instance.sendToPrimary(QByteArrayLiteral("settings"));
        }
        return 0;
    }

    QFont font(QStringLiteral("Inter"));
    if (!font.exactMatch()) {
        font = QFont(QStringLiteral("Segoe UI"));
    }
    if (!font.exactMatch()) {
        font = QFont(QStringLiteral("Noto Sans"));
    }
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(10);
    app.setFont(font);

    Application spaste;
    QObject::connect(&instance, &SingleInstance::messageReceived, &spaste,
                     [&spaste](const QByteArray& message) {
                         if (message == QByteArrayLiteral("toggle-overlay")) {
                             spaste.toggleOverlay();
                         } else {
                             spaste.showSettings();
                         }
                     });

    // First launch: settings. Pure --toggle-overlay: overlay only (no settings flash).
    const bool openSettings = wantSettings && !wantOverlay;
    if (!spaste.initialize(openSettings, wantOverlay)) {
        return 1;
    }

    // Normal launch opens settings; --toggle-overlay alone opens overlay.
    // If both were somehow set, overlay path already handled.
    if (wantSettings && wantOverlay) {
        spaste.showSettings();
    }

    return app.exec();
}
