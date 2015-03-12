#pragma once

#include <QtWidgets/QMainWindow>
#include <QtGui/QImage>
#include "ui_MainWindow.h"
#include "ColorCatalog.h"
#include <memory>
#include <QtWidgets/qtablewidget.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:

    virtual bool eventFilter(QObject *object, QEvent *qEvent) override;

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

    void DisplayColorCatalog(class QTableWidget *table);
    void GeneratePlan();
    void LoadImage();
    std::unique_ptr<double[]> getDataCost();

    void updatePlan(const class GCoptimizationGridGraph & gridGraph);
};