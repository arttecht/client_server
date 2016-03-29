#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>
#include "MyServer.h"
#include <QtAlgorithms>

Server::Server(QWidget *parent, qint32 port)
:   QDialog(parent), tcp_Port(port), tcpServer(0), networkSession(0)
{
    statusLabel = new QLabel;
    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    } else {
        sessionOpened();
    }

        connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(quitButton);
        buttonLayout->addStretch(1);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(statusLabel);
        mainLayout->addLayout(buttonLayout);
        setLayout(mainLayout);

        setWindowTitle(tr("Fortune Server"));
}

void Server::sessionOpened()
{
    // Save the used configuration
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any,tcp_Port)) {
        QMessageBox::critical(this, tr("Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Fortune Client example now.")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
}

void Server::slotNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    connect(clientConnection, SIGNAL(readyRead()),
            this,             SLOT(slotReadClient())
           );

    sendToClient(clientConnection, "Server Response: Connected");
}

#define M_DEBUG
void Server::slotReadClient()
{
    quint32     m_size = 0;
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);

    quint32     cnt = 0;
    QByteArray  data;
    in.setVersion(QDataStream::Qt_4_2);

    for (;;) {
        if (!m_size) {
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_size;
            cnt = 0;
        }

        if (pClientSocket->bytesAvailable() < m_size) {
            break;
        }

        QByteArray temp;
        in >> temp;

        if (!cnt) {
            QString str;
            qint32 i = 0;

            for (i = 0; i < temp.size(); ++i) {
                if (temp[i] == ' ') {
                    ++i; break;
                }
                str += temp[i];
            }
            m_size = m_size  - sizeof(quint32) - str.size() - 1;

#ifdef M_DEBUG
            qDebug() << "Fileame: " << str;
            qDebug() << "m_size: " << m_size;
            qDebug() << "str_size: " << str.size();
#endif
            file.setFileName(str);
            if (!file.open(QIODevice::WriteOnly)) {
                qDebug() << "File isn't created!";
                return;
            }

            for (qint32 j = i; j < temp.size(); ++j)
                data.push_back(temp[j]);

            file.write(data);
        }

        m_size = 0;
        cnt = 1;
        sendToClient(pClientSocket, "Server Response: Received");
    }
    file.close();
}

void Server::sendToClient(QTcpSocket* pSocket, const QString& str)
{
#ifdef M_DEBUG
    qDebug() << "sendToClient(): " << str;
#endif
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}
