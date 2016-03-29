#include <QApplication>
//#include <QtCore>
#include <stdlib.h>

#include "MyServer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server server(0, 22222);

    server.show();

//    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    return server.exec();
}
