#include <iostream>
#include <QApplication>
#include "MainWindow.h"

using namespace std;

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    application.exec();
    return 0;
}