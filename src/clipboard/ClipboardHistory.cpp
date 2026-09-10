#include "clipboard/ClipboardHistory.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QMimeData>
#include <QPixmap>

#include <algorithm>

QString ClipboardItem::fingerprint() const
{
    if (isImage()) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        const QImage img = image.convertToFormat(QImage::Format_ARGB32);
        hash.addData(reinterpret_cast<const char*>(img.constBits()), qsizetype(img.sizeInBytes()));
        return QStringLiteral("img:%1x%2:%3")
            .arg(img.width())
            .arg(img.height())
            .arg(QString::fromLatin1(hash.result().toHex()));
    }
    return QStringLiteral("txt:%1").arg(text);
}

ClipboardHistory::ClipboardHistory(QObject* parent)
    : QObject(parent)
{
    auto* clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &ClipboardHistory::onClipboardChanged);
    captureCurrentClipboard();
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
    m_lastFingerprint.clear();
    emit historyChanged();
}

void ClipboardHistory::onClipboardChanged()
{
    if (m_suppressNext) {
        m_suppressNext = false;
        return;
    }
    captureCurrentClipboard();
}

void ClipboardHistory::captureCurrentClipboard()
{
    const QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();
    if (!mime) {
        return;
    }

    ClipboardItem item;
    item.timestamp = QDateTime::currentDateTime();

    QImage image;
    if (mime->hasImage()) {
        image = qvariant_cast<QImage>(mime->imageData());
        if (image.isNull()) {
            const QPixmap pix = clipboard->pixmap();
            if (!pix.isNull()) {
                image = pix.toImage();
            }
        }
    }
    if (image.isNull()) {
        static const char* formats[] = {"image/png", "image/jpeg", "image/bmp", "image/webp", "image/tiff"};
        for (const char* format : formats) {
            if (mime->hasFormat(QLatin1String(format))) {
                image = QImage::fromData(mime->data(QLatin1String(format)));
                if (!image.isNull()) {
                    break;
                }
            }
        }
    }

    if (!image.isNull()) {
        item.kind = ClipboardKind::Image;
        item.image = image;
        if (mime->hasText()) {
            item.text = mime->text().trimmed();
        }
        addItem(std::move(item));
        return;
    }

    if (mime->hasText()) {
        const QString text = mime->text();
        if (text.isEmpty()) {
            return;
        }
        item.kind = ClipboardKind::Text;
        item.text = text;
        addItem(std::move(item));
    }
}

void ClipboardHistory::addItem(ClipboardItem item)
{
    const QString fp = item.fingerprint();
    if (fp == m_lastFingerprint && !m_items.isEmpty() && m_items.first().fingerprint() == fp) {
        return;
    }

    m_lastFingerprint = fp;

    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                                  [&](const ClipboardItem& existing) {
                                      return existing.fingerprint() == fp;
                                  }),
                  m_items.end());

    m_items.prepend(std::move(item));

    while (m_items.size() > kMaxItems) {
        m_items.removeLast();
    }

    emit historyChanged();
}
