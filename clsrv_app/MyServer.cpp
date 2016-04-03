#include <QtCore>
#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>
#include "MyServer.h"
#include <QtAlgorithms>

Server::Server(QWidget *parent, qint32 port)
:   QDialog(parent), tcp_Port(port), stateStartButton(0),
    tcpServer(0) , networkSession(0), m_size(0), setDir("/home")
{
    statusLabel = new QLabel;
    portLabel   = new QLabel;
    portLineEdit = new QLineEdit("22222");
    portLabel->setBuddy(portLineEdit);

    setFolder   = new QPushButton(tr("Set Folder"));
    startButton = new QPushButton(tr("Start Server"));
    quitButton  = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(setFolder, QDialogButtonBox::ActionRole);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(setFolder, SIGNAL(clicked()), this, SLOT(setStoreFolder()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(startServer()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    //    quitButton->setAutoDefault(false);
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
        if (sessionOpened() != SESSION_OK) {
            quitButton->animateClick(10);
        }
    }


    buttonBox->setMinimumHeight(30);
    portLineEdit->setMinimumWidth(20);

    mainLayout = new QGridLayout;
    mainLayout->addWidget(statusLabel, 0, 0, 1, 3);

    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);

    mainLayout->addWidget(buttonBox, 2, 0, 1, 0, Qt::AlignBottom);

    setLayout(mainLayout);
    setWindowTitle(tr("Fortune Server"));
}

Server::~Server()
{
    if (statusLabel != NULL) delete statusLabel;
    if (portLabel != NULL) delete portLabel;
    if (portLineEdit != NULL) delete portLineEdit;

    if (setFolder != NULL) delete setFolder;
    if (startButton != NULL) delete startButton;
    if (quitButton != NULL) delete quitButton;

    if (networkSession != NULL) delete networkSession;
    if (tcpServer != NULL) delete tcpServer;
}

void Server::setStoreFolder()
{
    qDebug() << "setStoreFolder()";
    QString curDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (curDir.size() != 0) {
        setDir.clear();
        setDir = curDir;
    }

   qDebug() << "Dir: " << setDir;
}

void Server::startServer()
{
    if (!stateStartButton)
    {
        stateStartButton = true;
        startButton->setText("Stop Server");
        portLineEdit->setDisabled(true);
    }
    else
    {
        stateStartButton = false;
        startButton->setText("Start Server");
        portLineEdit->setDisabled(false);

    }
}

int Server::sessionOpened()
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
        return SESSION_ERR;
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

    statusLabel->setText(tr("  The Server is ready to receive file(s)\n\n    IP:   \"%1\"").arg(ipAddress));
    portLabel->setText( tr("Port:") );
    portLineEdit->setText(tr("%1").arg(tcpServer->serverPort()));

    return SESSION_OK;
}

void Server::slotNewConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    sendToClient(clientConnection, "Server Response: Connected");
}

#define M_DEBUG
QString Server::takeFileName(QByteArray &data)
{
    QString str;
    for (qint32 i = 1; i < data[0] + 1; ++i)
    {
        str.push_back( char(data[i]) );
    }
    return str;
}


void Server::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    bool        cnt = false;

    in.setVersion(QDataStream::Qt_4_2);

    for (;;) {
        if (!m_size)
        {
            if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_size;
            cnt = false;
            qDebug() << "m_size" << m_size;
        }

        if (pClientSocket->bytesAvailable() < m_size) {
            break;
        }

        QByteArray data;
        in >> data;

        if (!cnt)
        {
            file.setFileName( takeFileName(data) );
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "File isn't created!";
                return;
            }

            m_size = m_size - sizeof(quint32) - data[0] - 1;
            file.write( data.right(m_size) );

#ifdef M_DEBUG
            qDebug() << "Fileame: " << takeFileName(data);
            qDebug() << "m_size: " << m_size;
            qDebug() << "str_size: " << quint32(data[0]);
#endif
        }
        m_size  = 0;
        cnt     = true;
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
