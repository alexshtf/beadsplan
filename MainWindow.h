#pragma once

#include <QtWidgets/QMainWindow>
#include <QtGui/QImage>
#include "ui_MainWindow.h"
#include "ColorCatalog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private:
    Ui_MainWindow _ui;
    QImage _image;
    ColorCatalog _colorCatalog;

    void GeneratePlan();

    void LoadImage();
};