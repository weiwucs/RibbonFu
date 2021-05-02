#include <QDebug>
#include <QtWidgets/QApplication>
#include <QtCore/QElapsedTimer>
#include <QtWidgets/QMainWindow>
#include "FramelessHelper.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QElapsedTimer cost;
    cost.start();
    QMainWindow mainWindow;
    FramelessHelper framelessHelper = new FramelessHelper(mainWindow);

    qDebug() <<"window build cost:"<<cost.elapsed()<<" ms";
    mainWindow.show();
    return app.exec();
}
