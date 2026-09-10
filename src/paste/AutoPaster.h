#pragma once

#include <QObject>
#include <QString>

class AutoPaster : public QObject {
    Q_OBJECT

public:
    explicit AutoPaster(QObject* parent = nullptr);

    // Copy text to the clipboard, then simulate Ctrl+V into the focused app.
    void copyAndPaste(const QString& text);

signals:
    void pasteAttempted(bool simulated);

private:
    void simulatePaste();
    [[nodiscard]] static bool pasteWithXTest();
    [[nodiscard]] static bool pasteWithExternalTool();
};
