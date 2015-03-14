#include "MainWindow.h"
#include "gco/GCoptimization.h"
#include "../../.clion10/system/cmake/generated/168ea010/168ea010/Release/ui_MainWindow.h"

#include <QtGui/QWheelEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/QGraphicsSceneWheelEvent>
#include <QtWidgets/QGraphicsPixmapItem>

#include <boost/numeric/conversion/cast.hpp>

namespace
{
    double sqr(double x) { return x * x; }

    void toYUV(double r, double g, double b, double& y, double& u, double& v)
    {
        y = 0.299    * r + 0.587   * g + 0.114   * b;
        u = -0.14713 * r - 0.28886 * g + 0.436   * b;
        v = 0.615    * r - 0.51499 * g - 0.10001 * b;
    }

    double ColorDiff(const QColor& left, const ColorCatalog::Entry& right)
    {
        double l[3];
        double r[3];

        toYUV(left.red(), left.green(), left.blue(), l[0], l[1], l[2]);
        toYUV(right.r, right.g, right.b, r[0], r[1], r[2]);

        return (fabs(l[0] - r[0]) + fabs(l[1] - r[1]) + fabs(l[2] - r[2])) / 255;
    }

    using SiteID = GCoptimization::SiteID;
    using LabelID = GCoptimization::LabelID;

    template<typename T>
    int AsInt(T x) { return boost::numeric_cast<int>(x); }

    constexpr double ZOOM_FACTOR = 1.1;
    constexpr double WHEEL_DELTA_FACTOR = 120.0;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    _ui.splitter->setStretchFactor(0, 0);
    _ui.splitter->setStretchFactor(1, 1);

    _ui.imgView->setScene(_imgScene = new QGraphicsScene());
    _ui.imgView->viewport()->installEventFilter(this);

    _ui.planView->setScene(_planScene = new QGraphicsScene());
    _ui.planView->viewport()->installEventFilter(this);

    DisplayColorCatalog(_ui.colorCatalogTable);

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
    auto planImage = _image.scaled(_ui.columnsSpinBox->value(), _ui.rowsSpinBox->value(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    try
    {
        GCoptimizationGridGraph gridGraph(
            static_cast<SiteID>(planImage.width()),
            static_cast<SiteID>(planImage.height()),
            static_cast<LabelID>(_colorCatalog.size())
        );

        auto dataCost = getDataCost(planImage);
        gridGraph.setDataCost(dataCost.get());
        gridGraph.expansion(100);
        updatePlan(planImage, gridGraph);
    }
    catch(const GCException& e)
    {
        QMessageBox::warning(this, tr("Unable to compute plan"), e.message);
    }
}

void MainWindow::updatePlan(const QImage &planImage, const class GCoptimizationGridGraph &gridGraph)
{
    QImage displayedPlan = planImage;
    for(SiteID x = 0; x < displayedPlan.width(); ++x)
        {
            for(SiteID y = 0; y < displayedPlan.height(); ++y)
            {
                auto id = y * displayedPlan.width() + x;
                auto label = gridGraph.whatLabel(id);
                auto& color = _colorCatalog.entryAt(static_cast<size_t>(label));
                displayedPlan.setPixel(x, y, qRgb(color.r, color.g, color.b));
            }
        }

    displayedPlan = displayedPlan.scaled(_image.width(), _image.height());
    _planPixmapItem.reset(_planScene->addPixmap(QPixmap::fromImage(displayedPlan)));
}

std::unique_ptr<double[]> MainWindow::getDataCost(const QImage& planImage)
{
    auto size = planImage.width() * planImage.height() * _colorCatalog.size();
    std::unique_ptr<double[]> dataCost(new double[size]);

    for(SiteID x = 0; x < planImage.width(); ++x)
    {
        for(SiteID y = 0; y < planImage.height(); ++y)
        {
            auto imgColor = QColor::fromRgb(planImage.pixel(x, y));
            auto siteId = y * planImage.width() + x;
            for(LabelID l = 0; l < _colorCatalog.size(); ++l)
            {
                auto& catalogColor = _colorCatalog.entryAt(static_cast<size_t>(l));
                dataCost[siteId * _colorCatalog.size() + l] = 64 * ColorDiff(imgColor, catalogColor);
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

void MainWindow::DisplayColorCatalog(QTableWidget *table)
{
    table->setRowCount(AsInt(_colorCatalog.size()));

    for(size_t i = 0; i < _colorCatalog.size(); ++i)
    {
        auto& entry = _colorCatalog.entryAt(i);

        std::unique_ptr<QTableWidgetItem> nameItem(new QTableWidgetItem);
        nameItem->setText(QString::fromStdString(entry.name));

        std::unique_ptr<QTableWidgetItem> colorItem(new QTableWidgetItem);
        colorItem->setText(QString("%1 %2 %3").arg(entry.r).arg(entry.g).arg(entry.b));
        colorItem->setBackground(QBrush(QColor::fromRgb(entry.r, entry.g, entry.b)));

        table->setItem(AsInt(i), 0, nameItem.release());
        table->setItem(AsInt(i), 1, colorItem.release());
    }
}
