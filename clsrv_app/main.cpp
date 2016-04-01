#include <QApplication>
#include <QtWidgets>

#include <stdlib.h>
#include "splitter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon( QIcon(":/icon_resources/icons/Actions-app.png"));
    Splitter mainapp;

    mainapp.show();

    return app.exec();
}
