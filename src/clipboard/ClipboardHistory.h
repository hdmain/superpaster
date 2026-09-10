#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QImage>

enum class ClipboardKind {
    Text,
    Image
};

struct ClipboardItem {
    ClipboardKind kind = ClipboardKind::Text;
    QString text;
    QImage image;
    QDateTime timestamp;

    [[nodiscard]] bool isImage() const { return kind == ClipboardKind::Image && !image.isNull(); }
    [[nodiscard]] bool isText() const { return kind == ClipboardKind::Text && !text.isEmpty(); }
    [[nodiscard]] QString fingerprint() const;
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
    void captureCurrentClipboard();
    void addItem(ClipboardItem item);

    QVector<ClipboardItem> m_items;
    QString m_lastFingerprint;
    bool m_suppressNext = false;
};
