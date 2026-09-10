#include "ui/ClipboardItemWidget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStyle>
#include <QSvgWidget>
#include <QVBoxLayout>

namespace {

QPixmap thumbnailFor(const QImage& image)
{
    constexpr int maxW = 96;
    constexpr int maxH = 72;
    return QPixmap::fromImage(image.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

} // namespace

ClipboardItemWidget::ClipboardItemWidget(const ClipboardItem& item, const QString& meta, QWidget* parent)
    : QFrame(parent)
    , m_item(item)
{
    setObjectName(QStringLiteral("ClipboardItem"));
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(12);

    if (m_item.isImage()) {
        auto* thumb = new QLabel(this);
        thumb->setObjectName(QStringLiteral("ItemThumb"));
        thumb->setFixedSize(96, 72);
        thumb->setAlignment(Qt::AlignCenter);
        thumb->setPixmap(thumbnailFor(m_item.image));
        thumb->setScaledContents(false);
        root->addWidget(thumb, 0, Qt::AlignTop);
    } else {
        auto* icon = new QSvgWidget(QStringLiteral(":/icons/copy.svg"), this);
        icon->setFixedSize(18, 18);
        root->addWidget(icon, 0, Qt::AlignTop);
    }

    auto* textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    textCol->setContentsMargins(0, 0, 0, 0);

    auto* preview = new QLabel(this);
    preview->setObjectName(QStringLiteral("ItemPreview"));
    preview->setWordWrap(true);
    preview->setTextInteractionFlags(Qt::NoTextInteraction);

    if (m_item.isImage()) {
        const QString sizeLabel = tr("Image %1x%2")
                                      .arg(m_item.image.width())
                                      .arg(m_item.image.height());
        if (!m_item.text.isEmpty()) {
            QString extra = m_item.text.simplified();
            if (extra.size() > 80) {
                extra = extra.left(77) + QStringLiteral("...");
            }
            preview->setText(sizeLabel + QStringLiteral("\n") + extra);
        } else {
            preview->setText(sizeLabel);
        }
    } else {
        QString text = m_item.text;
        text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        text = text.simplified();
        if (text.size() > 160) {
            text = text.left(157) + QStringLiteral("...");
        }
        preview->setText(text);
    }

    auto* metaLabel = new QLabel(meta, this);
    metaLabel->setObjectName(QStringLiteral("ItemMeta"));

    textCol->addWidget(preview);
    textCol->addWidget(metaLabel);
    root->addLayout(textCol, 1);
}

ClipboardItem ClipboardItemWidget::item() const
{
    return m_item;
}

void ClipboardItemWidget::setSelected(bool selected)
{
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void ClipboardItemWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit activated(m_item);
    }
    QFrame::mousePressEvent(event);
}

void ClipboardItemWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit activated(m_item);
    }
    QFrame::mouseDoubleClickEvent(event);
}

void ClipboardItemWidget::enterEvent(QEnterEvent* event)
{
    setSelected(true);
    QFrame::enterEvent(event);
}

void ClipboardItemWidget::leaveEvent(QEvent* event)
{
    setSelected(false);
    QFrame::leaveEvent(event);
}
