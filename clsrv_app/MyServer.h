#ifndef _MYSERVER_H
#define _MYSERVER_H

#include <QtWidgets>
#include <QDialog>
#include <QtNetwork>
#include <sys/stat.h>

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
    mode_t takeFileMode(QByteArray &data);
    void addListViewItem(const QString &str);

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
    QNetworkSession *networkSession;

    QFile file;
    quint32 m_size;
    QString setDir;
    QTime time;
    quint32 numOfFiles;
    quint32 totalBytes;

    QStringList fileList;
    QAbstractItemModel *model;
    QListView *listView;
};

#endif // _MYSERVER_H
