#include <QtGui>

#include "splitter.h"
#include "client.h"
#include "MyServer.h"

Splitter::Splitter(QWidget *parent) : QDialog(parent)
{
    statusLabel = new QLabel(tr("This application requires that you run \n"
                                "that one on the other host as well."));


    setClient = new QPushButton(tr("Client side"));
    setServer = new QPushButton(tr("Server side"));
    quitButton = new QPushButton(tr("Quit"));

    setClient->setEnabled(true);
    setServer->setEnabled(true);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(setClient, QDialogButtonBox::ActionRole);
    buttonBox->addButton(setServer, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(setClient,  SIGNAL(clicked()), this, SLOT(openClient()));
    connect(setServer,  SIGNAL(clicked()), this, SLOT(openServer()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    setClient->setMinimumSize(130,70);
    setClient->setIconSize(QSize(32,32));
    setClient->setIcon( QIcon(":/icon_resources/icons/Actions-files-2.png") );

    setServer->setMinimumSize(130,70);
    setServer->setIconSize(QSize(32,32));
    setServer->setIcon( QIcon(":/icon_resources/icons/Actions-drop-icon.png") );

    quitButton->setMinimumSize(130,70);
    quitButton->setIconSize(QSize(32,32));
    quitButton->setIcon( QIcon(":/icon_resources/icons/Actions-exit.png") );

    mainLayout = new QGridLayout;

    mainLayout->setRowMinimumHeight(0, 50);
    mainLayout->addWidget(statusLabel, 0, 0, 1, 0, Qt::AlignVCenter | Qt::AlignHCenter);

    mainLayout->setRowMinimumHeight(1, 80);
    mainLayout->addWidget(buttonBox, 1, 0, 2, 3, Qt::AlignHCenter);
    setLayout(mainLayout);

    setWindowTitle(tr("Client-Server filesender"));
}

Splitter::~Splitter()
{
    delete setClient;
    delete setServer;
    delete quitButton;

    delete buttonBox;
    delete mainLayout;
}

#define M_DEBUG

void Splitter::openClient()
{
#ifdef M_DEBUG
    qDebug() << "Splitter::openClient()";
#endif
    this->setVisible(false);

    QByteArray arr;
    arr = this->saveGeometry();

    Client client;
    client.exec();

    this->restoreGeometry(arr);
    this->show();
}

void Splitter::openServer()
{
#ifdef M_DEBUG
    qDebug() << "Splitter::openServer()";
#endif
    this->setVisible(false);

    QByteArray arr;
    arr = this->saveGeometry();

    Server server(this, 22222);
    qDebug() << "State Server: " << server.exec();

    this->restoreGeometry(arr);
    this->show();

}

