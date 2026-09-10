#include "ui/ClipboardItemWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QSvgWidget>
#include <QVBoxLayout>

ClipboardItemWidget::ClipboardItemWidget(const QString& text, const QString& meta, QWidget* parent)
    : QFrame(parent)
    , m_text(text)
{
    setObjectName(QStringLiteral("ClipboardItem"));
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(12);

    auto* icon = new QSvgWidget(QStringLiteral(":/icons/copy.svg"), this);
    icon->setFixedSize(18, 18);
    root->addWidget(icon, 0, Qt::AlignTop);

    auto* textCol = new QVBoxLayout();
    textCol->setSpacing(2);
    textCol->setContentsMargins(0, 0, 0, 0);

    m_preview = new QLabel(this);
    m_preview->setObjectName(QStringLiteral("ItemPreview"));
    m_preview->setWordWrap(true);
    m_preview->setTextInteractionFlags(Qt::NoTextInteraction);

    QString preview = text;
    preview.replace(QLatin1Char('\n'), QLatin1Char(' '));
    preview = preview.simplified();
    if (preview.size() > 160) {
        preview = preview.left(157) + QStringLiteral("...");
    }
    m_preview->setText(preview);

    auto* metaLabel = new QLabel(meta, this);
    metaLabel->setObjectName(QStringLiteral("ItemMeta"));

    textCol->addWidget(m_preview);
    textCol->addWidget(metaLabel);
    root->addLayout(textCol, 1);
}

QString ClipboardItemWidget::text() const
{
    return m_text;
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
        emit activated(m_text);
    }
    QFrame::mousePressEvent(event);
}

void ClipboardItemWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit activated(m_text);
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
