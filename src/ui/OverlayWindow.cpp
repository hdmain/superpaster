#include "ui/OverlayWindow.h"
#include "ui/ClipboardItemWidget.h"
#include "clipboard/ClipboardHistory.h"

#include <QApplication>
#include <QCursor>
#include <QFocusEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QScrollArea>
#include <QShowEvent>
#include <QSvgWidget>
#include <QTimer>
#include <QVBoxLayout>

OverlayWindow::OverlayWindow(ClipboardHistory* history, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_history(history)
{
    setObjectName(QStringLiteral("OverlayRoot"));
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(420, 520);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("OverlayPanel"));
    panel->setAttribute(Qt::WA_StyledBackground, true);

    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 18, 20, 18);
    panelLayout->setSpacing(14);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);

    auto* logo = new QSvgWidget(QStringLiteral(":/icons/clipboard.svg"), panel);
    logo->setFixedSize(28, 28);

    auto* title = new QLabel(QStringLiteral("Spaste"), panel);
    title->setObjectName(QStringLiteral("BrandTitle"));

    header->addWidget(logo);
    header->addWidget(title);
    header->addStretch();
    panelLayout->addLayout(header);

    auto* subtitle = new QLabel(tr("Recent clipboard — stored in memory only"), panel);
    subtitle->setObjectName(QStringLiteral("HintLabel"));
    panelLayout->addWidget(subtitle);

    m_scroll = new QScrollArea(panel);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFocusPolicy(Qt::NoFocus);

    auto* listHost = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(listHost);
    m_listLayout->setContentsMargins(0, 0, 4, 0);
    m_listLayout->setSpacing(8);
    m_listLayout->addStretch();

    m_emptyHint = new QLabel(tr("No copied items yet.\nCopy something to see it here."), listHost);
    m_emptyHint->setObjectName(QStringLiteral("EmptyHint"));
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_listLayout->insertWidget(0, m_emptyHint);

    m_scroll->setWidget(listHost);
    panelLayout->addWidget(m_scroll, 1);

    auto* footer = new QLabel(tr("↑↓ navigate · Enter paste · Esc close · Super+V toggle"), panel);
    footer->setObjectName(QStringLiteral("HintLabel"));
    footer->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(footer);

    root->addWidget(panel);

    connect(m_history, &ClipboardHistory::historyChanged, this, &OverlayWindow::rebuildList);
    qApp->installEventFilter(this);

    rebuildList();
}

void OverlayWindow::showOverlay()
{
    rebuildList();
    positionOnScreen();
    show();
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
    if (!m_widgets.isEmpty()) {
        selectIndex(0);
    }
}

void OverlayWindow::hideOverlay()
{
    hide();
    m_selectedIndex = -1;
}

void OverlayWindow::rebuildList()
{
    // Remove current rows but keep the empty-state label instance.
    QLayoutItem* item = nullptr;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            if (widget != m_emptyHint) {
                widget->deleteLater();
            }
        }
        delete item;
    }
    m_widgets.clear();

    const auto items = m_history->items();
    if (items.isEmpty()) {
        m_emptyHint->setVisible(true);
        m_listLayout->addWidget(m_emptyHint);
        m_listLayout->addStretch();
        m_selectedIndex = -1;
        return;
    }

    m_emptyHint->setVisible(false);

    for (const ClipboardItem& clip : items) {
        const QString meta = clip.timestamp.toString(QStringLiteral("HH:mm:ss"));
        auto* widget = new ClipboardItemWidget(clip.text, meta, m_scroll->widget());
        connect(widget, &ClipboardItemWidget::activated, this, [this](const QString& text) {
            emit itemChosen(text);
            hideOverlay();
        });
        m_listLayout->addWidget(widget);
        m_widgets.push_back(widget);
    }
    m_listLayout->addStretch();

    if (isVisible()) {
        selectIndex(0);
    } else {
        m_selectedIndex = -1;
    }
}

void OverlayWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        hideOverlay();
        break;
    case Qt::Key_Up:
        selectIndex(m_selectedIndex <= 0 ? m_widgets.size() - 1 : m_selectedIndex - 1);
        break;
    case Qt::Key_Down:
        selectIndex(m_selectedIndex + 1 >= m_widgets.size() ? 0 : m_selectedIndex + 1);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        activateSelected();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void OverlayWindow::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    // Delay hide slightly so item clicks still register
    QTimer::singleShot(120, this, [this]() {
        if (!isActiveWindow()) {
            hideOverlay();
        }
    });
}

bool OverlayWindow::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched)
    if (isVisible() && event->type() == QEvent::MouseButtonPress) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (!geometry().contains(mouse->globalPosition().toPoint())) {
            hideOverlay();
        }
    }
    return false;
}

void OverlayWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    positionOnScreen();
}

void OverlayWindow::positionOnScreen()
{
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    const QRect area = screen->availableGeometry();
    const int x = area.center().x() - width() / 2;
    const int y = area.center().y() - height() / 2;
    move(x, y);
}

void OverlayWindow::selectIndex(int index)
{
    if (m_widgets.isEmpty()) {
        m_selectedIndex = -1;
        return;
    }

    index = qBound(0, index, m_widgets.size() - 1);
    for (int i = 0; i < m_widgets.size(); ++i) {
        m_widgets[i]->setSelected(i == index);
    }
    m_selectedIndex = index;
    m_scroll->ensureWidgetVisible(m_widgets[index]);
}

void OverlayWindow::activateSelected()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_widgets.size()) {
        return;
    }
    emit itemChosen(m_widgets[m_selectedIndex]->text());
    hideOverlay();
}
