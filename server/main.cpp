#include <QApplication>
#include <stdlib.h>

#include "MyServer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server server(0, 22222);

    server.show();

    return server.exec();
}
