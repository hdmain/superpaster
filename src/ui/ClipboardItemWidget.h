#pragma once

#include "clipboard/ClipboardHistory.h"

#include <QFrame>

class QLabel;
class QMouseEvent;
class QEnterEvent;
class QEvent;

class ClipboardItemWidget : public QFrame {
    Q_OBJECT

public:
    explicit ClipboardItemWidget(const ClipboardItem& item, const QString& meta, QWidget* parent = nullptr);

    [[nodiscard]] ClipboardItem item() const;
    void setSelected(bool selected);

signals:
    void activated(const ClipboardItem& item);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    ClipboardItem m_item;
};
