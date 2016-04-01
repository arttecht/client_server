#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>
#include <QtDebug>
#include <QAbstractSocket>
#include <QFileDialog>

#include "splitter.h"
#include "client.h"

Client::Client(QWidget *parent)
:   QDialog(parent)
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
    fileLineEdit = new QLineEdit("vim.txt");
//    fileLineEdit = new QLineEdit();
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);
    fileLabel->setBuddy(fileLineEdit);

    statusLabel = new QLabel(tr("This examples requires that you run the "
                                "File Receiver Server example as well."));


    setFileButton = new QPushButton(tr("File Insert")); //***

    getFortuneButton = new QPushButton(tr("Send File"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(true);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(setFileButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    tcpSocket = new QTcpSocket(this);

    connect(hostLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableGetFortuneButton()));
    connect(fileLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableGetFortuneButton()));

    connect(setFileButton, SIGNAL(clicked()), this, SLOT(openFileDialog()));
    connect(getFortuneButton, SIGNAL(clicked()), this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(prepareAndSendData()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(fileLabel, 2, 0);     //***
    mainLayout->addWidget(fileLineEdit, 2, 1);  //***
    mainLayout->addWidget(statusLabel, 3, 0, 1, 3);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 3);
    setLayout(mainLayout);

    setWindowTitle(tr("File Sender Client"));
    portLineEdit->setFocus();
}

Client::~Client()
{
    delete hostLabel;
    delete portLabel;
    delete fileLabel;
    delete hostLineEdit;
    delete portLineEdit;
    delete fileLineEdit;
    delete statusLabel;

    delete getFortuneButton;
    delete quitButton;

    delete buttonBox;
    delete tcpSocket;
    delete mainLayout;
}

void Client::openFileDialog()
{
    QString str = QFileDialog::getOpenFileName(this, "Select File", "", "*");
    fileLineEdit->setText(str);
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

#define M_DEBUG

void Client::prepareFileName(QString &filename)
{
    bool modified = false;
    QString clear_filename;
    qint16 i = 0;

    for ( i = filename.length()-1; i >=0; --i ) {
        if (filename[i] == '\\' || filename[i] == '/') {
            modified = true;
            break;
        }
    }

    if (modified) {
        for (quint16 j = i+1; j < filename.length(); ++j ) {
            clear_filename.push_back(filename[j]);
        }
        filename.clear();
        filename = clear_filename;
    }
}



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
    QString f_name(fileLineEdit->text());

    prepareFileName(f_name);
#ifdef M_DEBUG
    qDebug() << "fileNameSize: " << f_name.length() << ", " << f_name;
#endif
    temp.append( char(f_name.length()) + f_name.toUtf8() );

    //Put the file content to array
    temp.append(file.readAll().data(), file.size());

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
    getFortuneButton->setEnabled(!hostLineEdit->text().isEmpty() &&
                                 !portLineEdit->text().isEmpty() &&
                                 !fileLineEdit->text().isEmpty());
}

