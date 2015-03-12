#pragma once

#include <QtWidgets/QMainWindow>
#include <QtGui/QImage>
#include "ui_MainWindow.h"
#include "ColorCatalog.h"
#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui_MainWindow _ui;
    QImage _image;
    ColorCatalog _colorCatalog;

    class QGraphicsScene* _imgScene;
    std::unique_ptr<class QGraphicsPixmapItem> _imgPixmapItem;

    class QGraphicsScene* _planScene;
    std::unique_ptr<class QGraphicsPixmapItem> _planPixmapItem;

    void GeneratePlan();
    void LoadImage();
    std::unique_ptr<double[]> getDataCost();

    void updatePlan(const class GCoptimizationGridGraph & gridGraph);
};