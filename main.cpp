#include "widget.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Chat window;

    Server my_server;

    window.show();

    return a.exec();
}
