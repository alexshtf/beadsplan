#include "MainWindow.h"
#include "gco/GCoptimization.h"

#include <QtGui/QWheelEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/QGraphicsSceneWheelEvent>
#include <QtWidgets/QGraphicsPixmapItem>

namespace
{
    double sqr(double x) { return x * x; }

    double ColorDiff(const QColor& left, const ColorCatalog::Entry& right)
    {
        auto leftR = left.red();
        auto leftG = left.green();
        auto leftB = left.blue();

        auto dR = (leftR - right.r) / 255.0;
        auto dG = (leftG - right.g) / 255.0;
        auto dB = (leftB - right.b) / 255.0;

        return sqrt(sqr(dR) + sqr(dG) + sqr(dB));
    }

    using SiteID = GCoptimization::SiteID;
    using LabelID = GCoptimization::LabelID ;

    constexpr double ZOOM_FACTOR = 1.1;
    constexpr double WHEEL_DELTA_FACTOR = 120.0;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);
    _ui.imgView->setScene(_imgScene = new QGraphicsScene());
    _ui.imgView->viewport()->installEventFilter(this);

    _ui.planView->setScene(_planScene = new QGraphicsScene());
    _ui.planView->viewport()->installEventFilter(this);

    connect(_ui.openImageButton, &QPushButton::pressed, [&] { LoadImage(); });
    connect(_ui.generatePlanButton, &QPushButton::pressed, [&] { GeneratePlan(); });
}

MainWindow::~MainWindow()
{
}

void MainWindow::LoadImage()
{
    QString imageFile = QFileDialog::getOpenFileName(this, tr("Open image"), QString(), tr("Images (*.png *.jpg *.xpm);;All files (*.*)"));
    if (!imageFile.isEmpty())
    {
        _image = QImage();
        if (_image.load(imageFile))
            _imgPixmapItem.reset(_imgScene->addPixmap(QPixmap::fromImage(_image)));
        else
            QMessageBox::warning(this, tr("Cannot open image"), imageFile);
    }
}

void MainWindow::GeneratePlan()
{
    try
    {
        GCoptimizationGridGraph gridGraph(
            static_cast<SiteID>(_image.width()),
            static_cast<SiteID>(_image.height()),
            static_cast<LabelID>(_colorCatalog.size())
        );

        auto dataCost = getDataCost();
        gridGraph.setDataCost(dataCost.get());
        gridGraph.expansion();
        updatePlan(gridGraph);
    }
    catch(const GCException& e)
    {
        QMessageBox::warning(this, tr("Unable to compute plan"), e.message);
    }
}

void MainWindow::updatePlan(const GCoptimizationGridGraph &gridGraph)
{
    for(SiteID x = 0; x < _image.width(); ++x)
        {
            for(SiteID y = 0; y < _image.height(); ++y)
            {
                auto id = y * _image.width() + x;
                auto label = gridGraph.whatLabel(id);
                auto& color = _colorCatalog.entryAt(static_cast<size_t>(label));
                _image.setPixel(x, y, qRgb(color.r, color.g, color.b));
            }
        }

    _planPixmapItem.reset(_planScene->addPixmap(QPixmap::fromImage(_image)));
}

std::unique_ptr<double[]> MainWindow::getDataCost()
{
    std::unique_ptr<double[]> dataCost(new double[_image.width() * _image.height() * _colorCatalog.size()]);
    for(SiteID x = 0; x < _image.width(); ++x)
    {
        for(SiteID y = 0; y < _image.height(); ++y)
        {
            auto imgColor = QColor::fromRgb(_image.pixel(x, y));
            auto siteId = y * _image.width() + x;
            for(LabelID l = 0; l < _colorCatalog.size(); ++l)
            {
                auto& catalogColor = _colorCatalog.entryAt(static_cast<size_t>(l));
                dataCost[siteId * _colorCatalog.size() + l] = 32 * ColorDiff(imgColor, catalogColor);
            }
        }
    }
    return dataCost;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    auto gfxView = qobject_cast<QGraphicsView*>(object->parent());
    if (event->type() == QEvent::Wheel)
    {
        auto wheelEvent = dynamic_cast<QWheelEvent*>(event);
        auto scale = std::pow(ZOOM_FACTOR, wheelEvent->angleDelta().y() / WHEEL_DELTA_FACTOR);

        gfxView->scale(scale, scale);
        return true;
    }
    if (event->type() == QEvent::GraphicsSceneWheel)
    {
        auto wheelEvent = dynamic_cast<QGraphicsSceneWheelEvent*>(event);
        auto scale = std::pow(ZOOM_FACTOR, wheelEvent->delta() / WHEEL_DELTA_FACTOR);

        gfxView->scale(scale, scale);
        return true;
    }

    return QObject::eventFilter(object, event);
}
