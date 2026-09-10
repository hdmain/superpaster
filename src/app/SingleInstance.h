#pragma once

#include <QObject>
#include <QString>

class QLocalServer;
class QLocalSocket;

class SingleInstance : public QObject {
    Q_OBJECT

public:
    static QString serverName();

    explicit SingleInstance(QObject* parent = nullptr);
    ~SingleInstance() override;

    [[nodiscard]] bool tryBecomePrimary();
    [[nodiscard]] bool sendToPrimary(const QByteArray& message);
    [[nodiscard]] bool isPrimary() const;

signals:
    void messageReceived(const QByteArray& message);

private:
    void onNewConnection();

    QLocalServer* m_server = nullptr;
    bool m_primary = false;
};
