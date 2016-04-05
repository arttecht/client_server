#ifndef _MYSERVER_H
#define _MYSERVER_H

#include <QtWidgets>
#include <QDialog>
#include <QtNetwork>

class QLabel;
class QPushButton;
class QTcpServer;
class QNetworkSession;

#define SESSION_OK  0
#define SESSION_ERR 1

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = 0, qint32 port = 0);
    ~Server();

    void openSession();
    void closeSession();
private slots:
    virtual void slotNewConnection();
    void slotReadClient();
    int sessionOpened();

    void setStoreFolder();
    void startServer();

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    QString takeFileName(QByteArray &data);

    qint32  tcp_Port;
    QLabel *statusLabel;
    QLabel *portLabel;
    QLabel *folderLabel;
    QLineEdit *portLineEdit;
    QLineEdit *folderLineEdit;

    QPushButton *setFolder;
    QPushButton *startButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QGridLayout *mainLayout;
    bool stateStartButton;

    QTcpServer *tcpServer;
    QStringList fortunes;
    QNetworkSession *networkSession;

    QFile file;
    quint32 m_size;
    QString setDir;
};

#endif // _MYSERVER_H
