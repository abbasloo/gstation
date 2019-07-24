#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize(1500, 800);
    w.show();
    w.setWindowTitle("DRONEASURE");

    return a.exec();
}
