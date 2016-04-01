#ifndef CLIENT_H
#define CLIENT_H

#include <QtWidgets>
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
    ~Client();

private slots:
    void requestNewFortune();
    void readFortune();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetFortuneButton();
    void prepareAndSendData();
    void openFileDialog();

private:
    void prepareFileName(QString &filename);

    QLabel *hostLabel;
    QLabel *portLabel;
    QLabel *fileLabel;

    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLineEdit *fileLineEdit;

    QLabel *statusLabel;
    QPushButton *setFileButton;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QGridLayout *mainLayout;

    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;
};


#endif
