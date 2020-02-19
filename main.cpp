#include "maincontroller.h"
#include <QApplication>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    int r;
    {
        QApplication a(argc, argv);
        MainController c;

        r = a.exec();
        qDebug() << "All QT stuff about to be closed";
    }
    qDebug() << "All QT stuff closed";

    return r;
}
