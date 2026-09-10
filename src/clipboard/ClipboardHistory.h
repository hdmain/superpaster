#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>

struct ClipboardItem {
    QString text;
    QDateTime timestamp;
};

class ClipboardHistory : public QObject {
    Q_OBJECT

public:
    static constexpr int kMaxItems = 10;

    explicit ClipboardHistory(QObject* parent = nullptr);

    [[nodiscard]] QVector<ClipboardItem> items() const;
    [[nodiscard]] int count() const;
    void clear();

public slots:
    void onClipboardChanged();

signals:
    void historyChanged();

private:
    void addItem(const QString& text);

    QVector<ClipboardItem> m_items;
    QString m_lastSeen;
    bool m_suppressNext = false;
};
