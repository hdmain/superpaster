#pragma once

#include "clipboard/ClipboardHistory.h"

#include <QObject>

class AutoPaster : public QObject {
    Q_OBJECT

public:
    explicit AutoPaster(QObject* parent = nullptr);

    // Put the item on the clipboard, then simulate Ctrl+V into the focused app.
    void copyAndPaste(const ClipboardItem& item);

signals:
    void pasteAttempted(bool simulated);

private:
    void simulatePaste();
    [[nodiscard]] static bool pasteWithXTest();
    [[nodiscard]] static bool pasteWithExternalTool();
};
