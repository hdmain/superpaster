#include "app/Application.h"

#include <QApplication>
#include <QFont>

int main(int argc, char* argv[])
{
    QApplication::setApplicationName(QStringLiteral("Spaste"));
    QApplication::setOrganizationName(QStringLiteral("Spaste"));
    QApplication::setApplicationVersion(QStringLiteral(SPASTE_VERSION));
    QApplication::setDesktopFileName(QStringLiteral("spaste"));

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

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
    if (!spaste.initialize()) {
        return 1;
    }

    return app.exec();
}
