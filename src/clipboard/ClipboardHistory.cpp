#include "clipboard/ClipboardHistory.h"

#include <QClipboard>
#include <QGuiApplication>

#include <algorithm>

ClipboardHistory::ClipboardHistory(QObject* parent)
    : QObject(parent)
{
    auto* clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardHistory::onClipboardChanged);

    const QString current = clipboard->text();
    if (!current.isEmpty()) {
        addItem(current);
    }
}

QVector<ClipboardItem> ClipboardHistory::items() const
{
    return m_items;
}

int ClipboardHistory::count() const
{
    return m_items.size();
}

void ClipboardHistory::clear()
{
    m_items.clear();
    m_lastSeen.clear();
    emit historyChanged();
}

void ClipboardHistory::onClipboardChanged()
{
    if (m_suppressNext) {
        m_suppressNext = false;
        return;
    }

    const QString text = QGuiApplication::clipboard()->text();
    if (text.isEmpty() || text == m_lastSeen) {
        return;
    }

    addItem(text);
}

void ClipboardHistory::addItem(const QString& text)
{
    m_lastSeen = text;

    // Drop duplicate if already at the front
    if (!m_items.isEmpty() && m_items.first().text == text) {
        return;
    }

    // Remove earlier duplicates of the same text
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                                  [&](const ClipboardItem& item) { return item.text == text; }),
                  m_items.end());

    m_items.prepend(ClipboardItem{text, QDateTime::currentDateTime()});

    while (m_items.size() > kMaxItems) {
        m_items.removeLast();
    }

    emit historyChanged();
}
