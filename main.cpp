#include "maincontroller.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    int r;
    {
        r = MainController(argc, argv).exec();
        qDebug() << "All QT stuff about to be closed";
    }
    qDebug() << "All QT stuff closed";

    return r;
}
