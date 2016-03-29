#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

private slots:
    void requestNewFortune();
    void readFortune();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetFortuneButton();
    void sessionOpened();
    void prepareAndSendData();

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QLabel *fileLabel;

    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *fileLineEdit;

    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;

    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;

    QNetworkSession *networkSession;
};

#endif
