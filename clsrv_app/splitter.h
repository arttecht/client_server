#ifndef SPLITTER_H
#define SPLITTER_H

#include <QtWidgets>
#include <QDialog>
#include <QTcpSocket>

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;


class Splitter : public QDialog
{
    Q_OBJECT

public:
    Splitter(QWidget *parent = 0);
    ~Splitter();

private slots:
    void openClient();
    void openServer();

private:
    QLabel *statusLabel;
    QPushButton *setClient;
    QPushButton *setServer;
    QPushButton *quitButton;

    QDialogButtonBox *buttonBox;
    QGridLayout *mainLayout;
};

#endif // SPLITTER_H
