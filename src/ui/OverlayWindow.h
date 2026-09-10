#pragma once

#include <QList>
#include <QWidget>

class ClipboardHistory;
class ClipboardItemWidget;
class QVBoxLayout;
class QLabel;
class QScrollArea;
class QKeyEvent;
class QFocusEvent;
class QShowEvent;

class OverlayWindow : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWindow(ClipboardHistory* history, QWidget* parent = nullptr);

public slots:
    void showOverlay();
    void hideOverlay();
    void rebuildList();

signals:
    void itemChosen(const QString& text);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void positionOnScreen();
    void selectIndex(int index);
    void activateSelected();

    ClipboardHistory* m_history = nullptr;
    QVBoxLayout* m_listLayout = nullptr;
    QLabel* m_emptyHint = nullptr;
    QScrollArea* m_scroll = nullptr;
    QList<ClipboardItemWidget*> m_widgets;
    int m_selectedIndex = -1;
};
