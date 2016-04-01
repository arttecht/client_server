#ifndef _MYSERVER_H
#define _MYSERVER_H

#include <QtWidgets>
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
    ~Server();

private slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void sessionOpened();

    void setStoreFolder();
    void startServer();

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    QString takeFileName(QByteArray &data);

    qint32  tcp_Port;
    QLabel *statusLabel;
    QLabel *portLabel;
    QLineEdit *portLineEdit;

    QPushButton *setFolder;
    QPushButton *startButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QGridLayout *mainLayout;

    QTcpServer *tcpServer;
    QStringList fortunes;
    QNetworkSession *networkSession;

    QFile file;
    quint32 m_size;
};

#endif // _MYSERVER_H
