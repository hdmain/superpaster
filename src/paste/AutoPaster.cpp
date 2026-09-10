#include "paste/AutoPaster.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#if defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

AutoPaster::AutoPaster(QObject* parent)
    : QObject(parent)
{
}

void AutoPaster::copyAndPaste(const ClipboardItem& item)
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (item.isImage()) {
        clipboard->setImage(item.image);
    } else {
        clipboard->setText(item.text);
    }

    // Give the previously focused window time to regain focus after the overlay hides.
    QTimer::singleShot(100, this, [this]() {
        simulatePaste();
    });
}

void AutoPaster::simulatePaste()
{
    bool ok = false;
#if defined(Q_OS_LINUX)
    ok = pasteWithXTest();
    if (!ok) {
        ok = pasteWithExternalTool();
    }
#endif
    if (!ok) {
        qWarning() << "Spaste: copied to clipboard, but could not simulate Ctrl+V"
                    << "(install xdotool or wtype for Wayland paste)";
    }
    emit pasteAttempted(ok);
}

bool AutoPaster::pasteWithXTest()
{
#if defined(Q_OS_LINUX)
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return false;
    }

    int eventBase = 0;
    int errorBase = 0;
    int major = 0;
    int minor = 0;
    if (!XTestQueryExtension(display, &eventBase, &errorBase, &major, &minor)) {
        XCloseDisplay(display);
        return false;
    }

    const KeyCode control = XKeysymToKeycode(display, XK_Control_L);
    const KeyCode vKey = XKeysymToKeycode(display, XK_v);
    if (control == 0 || vKey == 0) {
        XCloseDisplay(display);
        return false;
    }

    XTestFakeKeyEvent(display, control, True, CurrentTime);
    XTestFakeKeyEvent(display, vKey, True, CurrentTime);
    XTestFakeKeyEvent(display, vKey, False, CurrentTime);
    XTestFakeKeyEvent(display, control, False, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
    return true;
#else
    return false;
#endif
}

bool AutoPaster::pasteWithExternalTool()
{
    struct Candidate {
        QString binary;
        QStringList args;
    };

    const Candidate candidates[] = {
        {QStringLiteral("wtype"), {QStringLiteral("-M"), QStringLiteral("ctrl"), QStringLiteral("v"),
                                   QStringLiteral("-m"), QStringLiteral("ctrl")}},
        {QStringLiteral("xdotool"),
         {QStringLiteral("key"), QStringLiteral("--clearmodifiers"), QStringLiteral("ctrl+v")}},
        {QStringLiteral("ydotool"),
         {QStringLiteral("key"), QStringLiteral("29:1"), QStringLiteral("47:1"), QStringLiteral("47:0"),
          QStringLiteral("29:0")}},
    };

    for (const Candidate& candidate : candidates) {
        const QString path = QStandardPaths::findExecutable(candidate.binary);
        if (path.isEmpty()) {
            continue;
        }
        const int code = QProcess::execute(path, candidate.args);
        if (code == 0) {
            return true;
        }
        qWarning() << "Spaste:" << candidate.binary << "exited with" << code;
    }
    return false;
}
