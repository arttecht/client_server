#include <QtCore>
#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>
#include "MyServer.h"
#include <QtAlgorithms>
#include "splitter.h"
#include <sys/stat.h>


Server::Server(QWidget *parent, qint32 port)
:   QDialog(parent), tcp_Port(port), stateStartButton(0),
    tcpServer(0) , networkSession(0), m_size(0), setDir("/home"),
    model(0)
{

    statusLabel = new QLabel(tr("  The Server is ready to configure\n\n    IP:   \"aaa.bbb.ccc.ddd\""));
    portLabel   = new QLabel( tr("Port:") );
    folderLabel   = new QLabel(tr("Folder:"));
    portLineEdit = new QLineEdit(tr("%1").arg(tcp_Port));
    folderLineEdit = new QLineEdit(setDir);
    portLabel->setBuddy(portLineEdit);

    setFolder   = new QPushButton(tr("Set Store Folder"));
    startButton = new QPushButton(tr("Start Server"));
    quitButton  = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(setFolder, QDialogButtonBox::ActionRole);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(setFolder, SIGNAL(clicked()), this, SLOT(setStoreFolder()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(startServer()));

    buttonBox->setMinimumHeight(30);
    buttonBox->setMinimumWidth(600);
    portLineEdit->setMinimumWidth(20);


    listView = new QListView;

    mainLayout = new QGridLayout;
    mainLayout->addWidget(statusLabel, 0, 0, 1, 3);

    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);

    mainLayout->addWidget(folderLabel, 2, 0);
    mainLayout->addWidget(folderLineEdit, 2, 1);

    mainLayout->addWidget(buttonBox, 3, 0, 1, 0, Qt::AlignBottom);
    mainLayout->addWidget(listView, 4, 0, 1, 0);

    setLayout(mainLayout);
    setWindowTitle(tr("Fortune Server"));
}

Server::~Server()
{
    delete statusLabel;
    delete portLabel;
    delete portLineEdit;

    delete folderLabel;
    delete folderLineEdit;

    delete setFolder;
    delete startButton;
    delete quitButton;

    delete networkSession;

    if (tcpServer != NULL) {
        delete tcpServer;
    }
    delete model;
    delete listView;
}

void Server::setStoreFolder()
{
#ifdef M_DEBUG
    qDebug() << "setStoreFolder()";
#endif
    QString curDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (curDir.size() != 0) {
        setDir.clear();
        setDir = curDir;
    }
    folderLineEdit->setText(setDir);
#ifdef M_DEBUG
    qDebug() << "Dir: " << setDir;
#endif
}

void Server::startServer()
{
    if (!stateStartButton)
    {
        stateStartButton = true;
        startButton->setText("Stop Server");
        portLineEdit->setDisabled(true);
        folderLineEdit->setDisabled(true);
        setFolder->setDisabled(true);

        tcp_Port = portLineEdit->text().toInt();
        openSession();

        fileList.clear();
        addListViewItem("");
    }
    else
    {
        stateStartButton = false;
        startButton->setText("Start Server");
        portLineEdit->setDisabled(false);
        folderLineEdit->setDisabled(false);
        setFolder->setDisabled(false);

        statusLabel->setText(tr("  The Server is interrupted\n\n    IP:   \"aaa.bbb.ccc.ddd\""));
        closeSession();
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
    portLineEdit->setText(tr("%1").arg(tcpServer->serverPort()));

    return SESSION_OK;
}


void Server::slotNewConnection()
{
#ifdef M_DEBUG
    qDebug() << "slotNewConnection()";
#endif
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    sendToClient(clientConnection, "Server Response: Connected");
    numOfFiles = 0;
    m_size = 0;
    totalBytes = 0;
    time.start();
}

QString Server::takeFileName(QByteArray &data)
{
#ifdef M_DEBUG
    qDebug() << "takeFileName()";
#endif
    QString str = setDir;

    if (str[0] == '\\')
        str.push_back('\\');
    else
        str.push_back('/');

    for (qint32 i = 1; i < data[0] + 1; ++i)
    {
        str.push_back( char(data[i]) );
    }
#ifdef M_DEBUG
    qDebug() << str;
#endif
    return str;
}

mode_t Server::takeFileMode(QByteArray &data)
{
#ifdef M_DEBUG
    qDebug() << "takeFileMode()";
#endif
    const char* a_data = data.constData();
    mode_t f_mode = *( (mode_t*)&a_data[ a_data[0]+1 ] );

    return f_mode;
}

void Server::addListViewItem(const QString &str)
{
    if (model)
        delete model;

    if (str.size() != 0) {
        fileList.append(str);
    }
    model = new QStringListModel(fileList);
    listView->setModel(model);

}

void Server::slotReadClient()
{
#ifdef M_DEBUG
    qDebug() << "slotReadClient()";
#endif
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
#ifdef M_DEBUG
            qDebug() << "m_size" << m_size;
#endif
        }

        if (pClientSocket->bytesAvailable() < m_size) {
            break;
        }

        QByteArray data;
        in >> data;

        if (m_size == 0 && data.size() == 1)
        {
            qint32 parcelTime = time.elapsed();
            addListViewItem("======================================================");
            addListViewItem(tr("Passed %1 file(s); Elapsed time: %2 msec; Total: %3 bytes.")
                            .arg(numOfFiles)
                            .arg(parcelTime)
                            .arg(totalBytes));
            sendToClient(pClientSocket, "Server Response: Finalization Accepted");
            break;
        }

        if (!cnt)
        {
            QString fileName = takeFileName(data);

            file.setFileName( fileName );
            if (!file.open(QIODevice::WriteOnly))
            {
#ifdef M_DEBUG
                qDebug() << "File isn't created!";
#endif
                m_size  = 0;
                return;
            }

            mode_t f_mode = takeFileMode(data);
            chmod(fileName.toStdString().c_str(), f_mode);

            //Calculate clear file size
            m_size = m_size - sizeof(quint32) - sizeof(mode_t) - data[0] - 1;
            totalBytes += m_size;

            fileName.append(tr("; [%1 bytes]").arg(m_size));
            addListViewItem(fileName);

            file.write( data.right(m_size) );

#ifdef M_DEBUG
            qDebug() << "Fileame: " << fileName;
            qDebug() << "m_size: " << m_size;
            qDebug() << "str_size: " << quint32(data[0]);
#endif
        }

        m_size  = 0;
        cnt     = true;
        numOfFiles++;
        sendToClient(pClientSocket, "Server Response: Received");

        data.clear();
        file.close();
    }
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

void Server::openSession()
{
#ifdef M_DEBUG
    qDebug() << "openSession()";
#endif
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
//            quitButton->animateClick(10);
        }
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

void Server::closeSession()
{
    if (tcpServer) {
        tcpServer->close();
        delete tcpServer;
        tcpServer = 0;
    }
}
