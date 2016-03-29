#ifndef _MYSERVER_H
#define _MYSERVER_H

#include <QDialog>
#include <QtNetwork>

class QLabel;
class QPushButton;
class QTcpServer;
class QNetworkSession;

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = 0, qint32 port = 0);

public slots:
private slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void sessionOpened();

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

    qint32  tcp_Port;
    QLabel *statusLabel;
    QPushButton *quitButton;
    QTcpServer *tcpServer;
    QStringList fortunes;
    QNetworkSession *networkSession;
    QFile file;
};

#endif // _MYSERVER_H
