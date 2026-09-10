#pragma once

#include <QFrame>
#include <QString>

class QLabel;
class QMouseEvent;
class QEnterEvent;
class QEvent;

class ClipboardItemWidget : public QFrame {
    Q_OBJECT

public:
    explicit ClipboardItemWidget(const QString& text, const QString& meta, QWidget* parent = nullptr);

    [[nodiscard]] QString text() const;
    void setSelected(bool selected);

signals:
    void activated(const QString& text);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_text;
    QLabel* m_preview = nullptr;
};
