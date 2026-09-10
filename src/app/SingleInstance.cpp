#include "app/SingleInstance.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>

SingleInstance::SingleInstance(QObject* parent)
    : QObject(parent)
{
}

SingleInstance::~SingleInstance()
{
    if (m_server) {
        m_server->close();
        QLocalServer::removeServer(serverName());
    }
}

QString SingleInstance::serverName()
{
    return QStringLiteral("spaste-ipc");
}

bool SingleInstance::tryBecomePrimary()
{
    QLocalSocket probe;
    probe.connectToServer(serverName());
    if (probe.waitForConnected(150)) {
        probe.disconnectFromServer();
        m_primary = false;
        return false;
    }

    QLocalServer::removeServer(serverName());
    m_server = new QLocalServer(this);
    if (!m_server->listen(serverName())) {
        qWarning("Spaste: could not create IPC server: %s", qPrintable(m_server->errorString()));
        delete m_server;
        m_server = nullptr;
        m_primary = false;
        return false;
    }

    connect(m_server, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
    m_primary = true;
    return true;
}

bool SingleInstance::sendToPrimary(const QByteArray& message)
{
    QLocalSocket socket;
    socket.connectToServer(serverName());
    if (!socket.waitForConnected(500)) {
        return false;
    }
    socket.write(message + '\n');
    socket.flush();
    socket.waitForBytesWritten(500);
    socket.waitForReadyRead(100);
    socket.disconnectFromServer();
    return true;
}

bool SingleInstance::isPrimary() const
{
    return m_primary;
}

void SingleInstance::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket* socket = m_server->nextPendingConnection();
        auto tryRead = [this, socket]() {
            const QByteArray payload = socket->readAll().trimmed();
            if (!payload.isEmpty()) {
                emit messageReceived(payload);
            }
        };
        if (socket->bytesAvailable() > 0) {
            tryRead();
        }
        connect(socket, &QLocalSocket::readyRead, this, tryRead);
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    }
}
