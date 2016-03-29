#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>
#include <QtDebug>
#include <QAbstractSocket>
//#include <QThread>

#include "client.h"

Client::Client(QWidget *parent)
:   QDialog(parent), networkSession(0)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));
    fileLabel = new QLabel(tr("File name:"));

    // find out which IP to connect to
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

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit = new QLineEdit("22222");
//    fileLineEdit = new QLineEdit("../test_files/aaa.txt");
    fileLineEdit = new QLineEdit();
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);
    fileLabel->setBuddy(fileLineEdit);

    statusLabel = new QLabel(tr("This examples requires that you run the "
                                "Fortune Server example as well."));

    getFortuneButton = new QPushButton(tr("Send File"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(true);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(fileLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));

    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(prepareAndSendData()));

    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(fileLabel, 2, 0);     //***
    mainLayout->addWidget(fileLineEdit, 2, 1);  //***
    mainLayout->addWidget(statusLabel, 3, 0, 1, 3);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 3);
    setLayout(mainLayout);

    setWindowTitle(tr("Fortune Client"));
    portLineEdit->setFocus();

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

        getFortuneButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }
}

void Client::requestNewFortune()
{
    getFortuneButton->setEnabled(false);
    blockSize = 0;

    tcpSocket->abort();
    tcpSocket->connectToHost(hostLineEdit->text(),
                             portLineEdit->text().toInt());

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
}

//#define M_DEBUG
void Client::prepareAndSendData()
{
#ifdef M_DEBUG
    qDebug() << "fileNameSize: " << fileLineEdit->text().length() << ", " << fileLineEdit->text();
#endif
    QFile file(fileLineEdit->text());
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Fortune Client"), tr("The file is not found"));
        return;
    }

    QByteArray  arrBlock, temp;
    QDataStream out(&arrBlock, QIODevice::ReadWrite);

    out.setVersion(QDataStream::Qt_4_2);

    //Take the clear filename without path
    QString clear_fname, f_name(fileLineEdit->text());
    quint16 i = 0, modified = 0;

    for ( i = f_name.length()-1; i >=0; --i ){
        if (f_name[i] == '\\' || f_name[i] == '/') {
            modified = 1;
            break;
        }
    }

    if (modified == 1) {
        for (quint16 j = i+1; j < f_name.length(); ++j ){
            clear_fname += f_name[j];
        }
        temp.append(clear_fname.toUtf8());
    }
    else {
        temp.append(fileLineEdit->text().toUtf8());
    }

    temp.append(' ');

    //Put the file content to array
    quint32 f_len = file.readAll().length();
    file.seek(0);
    temp.append(file.readAll().data(), f_len);

    out << quint32(0) << temp;

    out.device()->seek(0);
    out << quint32(arrBlock.size() - sizeof(quint32));

    tcpSocket->write(arrBlock);

#ifdef M_DEBUG
    qDebug() << "arrBlock Size: " << arrBlock.length();
#endif
}

void Client::readFortune()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    blockSize = 0;
    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;

        in >> blockSize;
    }

    if (tcpSocket->bytesAvailable() < blockSize)
        return;

    in >> currentFortune;

#ifdef M_DEBUG
    qDebug() << "blocksize: " << blockSize << ", " << currentFortune;
#endif

    statusLabel->setText(currentFortune);
    getFortuneButton->setEnabled(true);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
    getFortuneButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty() &&
                                 !fileLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // Save the used configuration
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

    statusLabel->setText(tr("This examples requires that you run the "
                            "Fortune Server example as well."));

    enableGetFortuneButton();
}
