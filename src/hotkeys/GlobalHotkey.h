#pragma once

#include <QObject>
#include <memory>

class GlobalHotkey : public QObject {
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject* parent = nullptr);
    ~GlobalHotkey() override;

    bool registerHotkey();
    void unregisterHotkey();

signals:
    void activated();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    bool m_registered = false;
};
