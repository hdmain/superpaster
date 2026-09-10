#include "hotkeys/GlobalHotkey.h"

#include <QDebug>
#include <QSocketNotifier>

#if defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

class GlobalHotkey::Impl {
public:
#if defined(Q_OS_LINUX)
    Display* display = nullptr;
    Window root = 0;
    int keycode = 0;
    unsigned int modifiers = Mod4Mask;
    unsigned int numlock = 0;
    QSocketNotifier* notifier = nullptr;
#endif
};

namespace {

#if defined(Q_OS_LINUX)
unsigned int detectNumlockMask(Display* display)
{
    XModifierKeymap* modmap = XGetModifierMapping(display);
    if (!modmap) {
        return 0;
    }

    const KeyCode numlock = XKeysymToKeycode(display, XK_Num_Lock);
    unsigned int mask = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < modmap->max_keypermod; ++j) {
            if (modmap->modifiermap[i * modmap->max_keypermod + j] == numlock) {
                mask = 1u << i;
            }
        }
    }
    XFreeModifiermap(modmap);
    return mask;
}

void grabAll(Display* display, Window root, int keycode, unsigned int modifiers, unsigned int numlock)
{
    const unsigned int extras[] = {0u, LockMask, numlock, LockMask | numlock};
    for (unsigned int extra : extras) {
        XGrabKey(display, keycode, modifiers | extra, root, True, GrabModeAsync, GrabModeAsync);
    }
}

void ungrabAll(Display* display, Window root, int keycode, unsigned int modifiers, unsigned int numlock)
{
    const unsigned int extras[] = {0u, LockMask, numlock, LockMask | numlock};
    for (unsigned int extra : extras) {
        XUngrabKey(display, keycode, modifiers | extra, root);
    }
}
#endif

} // namespace

GlobalHotkey::GlobalHotkey(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
}

GlobalHotkey::~GlobalHotkey()
{
    unregisterHotkey();
}

bool GlobalHotkey::registerHotkey()
{
#if defined(Q_OS_LINUX)
    if (m_registered) {
        return true;
    }

    m_impl->display = XOpenDisplay(nullptr);
    if (!m_impl->display) {
        qWarning() << "Spaste: cannot open X11 display - Super+V unavailable "
                      "(on Wayland, bind Super+V in your compositor or run under XWayland)";
        return false;
    }

    m_impl->root = DefaultRootWindow(m_impl->display);
    m_impl->keycode = XKeysymToKeycode(m_impl->display, XK_v);
    m_impl->modifiers = Mod4Mask;
    m_impl->numlock = detectNumlockMask(m_impl->display);

    grabAll(m_impl->display, m_impl->root, m_impl->keycode, m_impl->modifiers, m_impl->numlock);
    XFlush(m_impl->display);

    const int fd = ConnectionNumber(m_impl->display);
    m_impl->notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_impl->notifier, &QSocketNotifier::activated, this, [this]() {
        if (!m_impl->display) {
            return;
        }
        while (XPending(m_impl->display)) {
            XEvent event;
            XNextEvent(m_impl->display, &event);
            if (event.type == KeyPress) {
                const unsigned int mods = event.xkey.state
                    & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);
                if (static_cast<int>(event.xkey.keycode) == m_impl->keycode && (mods & Mod4Mask)) {
                    emit activated();
                }
            }
        }
    });

    m_registered = true;
    return true;
#else
    qWarning() << "Spaste: global hotkeys are implemented for Linux/X11 only";
    return false;
#endif
}

void GlobalHotkey::unregisterHotkey()
{
#if defined(Q_OS_LINUX)
    if (!m_registered) {
        return;
    }

    if (m_impl->notifier) {
        m_impl->notifier->setEnabled(false);
        delete m_impl->notifier;
        m_impl->notifier = nullptr;
    }

    if (m_impl->display) {
        ungrabAll(m_impl->display, m_impl->root, m_impl->keycode, m_impl->modifiers, m_impl->numlock);
        XCloseDisplay(m_impl->display);
        m_impl->display = nullptr;
    }

    m_registered = false;
#endif
}
