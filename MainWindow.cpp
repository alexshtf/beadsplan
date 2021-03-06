#include "MainWindow.h"
#include "gco/GCoptimization.h"
#include "../../.clion10/system/cmake/generated/168ea010/168ea010/Release/ui_MainWindow.h"

#include <QtGui/QWheelEvent>
#include <QtGui/QPicture>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/QGraphicsSceneWheelEvent>
#include <QtWidgets/QGraphicsPixmapItem>

#include <boost/numeric/conversion/cast.hpp>

namespace
{
    double sqr(double x) { return x * x; }
    double ratio(double x, double y) { return x / y; }
    int roundInt(double x) { return static_cast<int>(std::round(x)); }

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

    QColor GetAverageColor(const QImage& img, int x, int y, int w, int h)
    {
        double
            avgR = 0,
            avgG = 0,
            avgB = 0;
        for(int dx = 0; dx < w; ++dx)
        {
            for(int dy = 0; dy < h; ++dy)
            {
                if (x + dx >= img.width())
                    continue;
                if (y + dy >= img.height())
                    continue;

                auto c = QColor::fromRgb(img.pixel(x + dx, y + dy));
                avgR += c.red();
                avgG += c.green();
                avgB += c.blue();
            }
        }

        avgR /= w * h;
        avgG /= w * h;
        avgB /= w * h;

        return QColor::fromRgb(static_cast<int>(avgR), static_cast<int>(avgG), static_cast<int>(avgB));
    }

    struct DataCostHelper
    {
    private:
        const QImage& _img;
        int _blockW;
        int _blockH;
    public:
        DataCostHelper(const QImage& img, int blockW, int blockH)
            : _img(img), _blockW(blockW), _blockH(blockH)
        {
        }

        double GetCost(int x, int y, const ColorCatalog::Entry& entry)
        {
            auto avgColor = GetAverageColor(_img, x, y, _blockW, _blockH);
            return ColorDiff(avgColor, entry);
        }
    };

    using SiteID = GCoptimization::SiteID;
    using LabelID = GCoptimization::LabelID;

    template<typename T>
    int AsInt(T x) { return boost::numeric_cast<int>(x); }

    constexpr double ZOOM_FACTOR = 1.1;
    constexpr double WHEEL_DELTA_FACTOR = 120.0;
    constexpr double PLAN_DISPLAY_CELL_SIZE = 10;
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
    connect(_ui.columnsSpinBox, &QSpinBox::editingFinished, [&] { UpdateRowsFromColumns(); });
    connect(_ui.rowsSpinBox, &QSpinBox::editingFinished, [&] { UpdateColumnsFromRows(); });
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
        {
            _imgPixmapItem.reset(_imgScene->addPixmap(QPixmap::fromImage(_image)));
            UpdateColumnsFromRows();
        }
        else
            QMessageBox::warning(this, tr("Cannot open image"), imageFile);
    }
}

void MainWindow::GeneratePlan()
{
    auto planWidth = _ui.columnsSpinBox->value();
    auto planHeight = _ui.rowsSpinBox->value();
    try
    {
        GCoptimizationGridGraph gridGraph(
            static_cast<SiteID>(planWidth),
            static_cast<SiteID>(planHeight),
            static_cast<LabelID>(_colorCatalog.size())
        );

        auto dataCost = GetDataCost(_image, planWidth, planHeight);
        gridGraph.setDataCost(dataCost.get());
        gridGraph.expansion(100);
        updatePlan(gridGraph, planWidth, planHeight);
    }
    catch(const GCException& e)
    {
        QMessageBox::warning(this, tr("Unable to compute plan"), e.message);
    }
}

void MainWindow::updatePlan(const class GCoptimizationGridGraph &gridGraph, int planWidth, int planHeight)
{
    auto xRatio = PLAN_DISPLAY_CELL_SIZE;
    auto yRatio = PLAN_DISPLAY_CELL_SIZE;

    QPixmap displayedPlan(static_cast<int>(xRatio * planWidth), static_cast<int>(yRatio * planHeight));
    QPainter painter(&displayedPlan);

    painter.setPen(Qt::black);
    for(SiteID x = 0; x < planWidth; ++x)
        {
            for(SiteID y = 0; y < planHeight; ++y)
            {
                auto id = y * planWidth + x;
                auto label = gridGraph.whatLabel(id);
                auto& entry = _colorCatalog.entryAt(static_cast<size_t>(label));

                painter.setBrush(QColor::fromRgb(entry.r, entry.g, entry.b));
                painter.drawRect(QRectF(x * xRatio, y * yRatio, xRatio, yRatio));
            }
        }

    _planPixmapItem.reset(_planScene->addPixmap(displayedPlan));
    _planPixmapItem->setTransformationMode(Qt::SmoothTransformation);
}

std::unique_ptr<double[]> MainWindow::GetDataCost(const QImage &planImage, int planWidth, int planHeight)
{
    auto size = planWidth * planHeight * _colorCatalog.size();
    std::unique_ptr<double[]> dataCost(new double[size]);

    auto dX = ratio(planImage.width(), planWidth);
    auto dY = ratio(planImage.height(), planHeight);
    DataCostHelper helper(planImage, roundInt(dX), roundInt(dY));

    for(SiteID x = 0; x < planWidth; ++x)
    {
        for(SiteID y = 0; y < planHeight; ++y)
        {
            auto siteId = y * planWidth + x;
            for(LabelID l = 0; l < _colorCatalog.size(); ++l)
            {
                auto& catalogColor = _colorCatalog.entryAt(static_cast<size_t>(l));
                dataCost[siteId * _colorCatalog.size() + l] = 64 * helper.GetCost(roundInt(x * dX), roundInt(y * dY), catalogColor);
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


void MainWindow::UpdateRowsFromColumns()
{
    if (_image.isNull())
        return;

    if (!_ui.keepAspectRatioCheckBox->isChecked())
        return;

    auto ratio = _image.width() / static_cast<double>(_image.height());
    _ui.rowsSpinBox->setValue(static_cast<int>(std::round(_ui.columnsSpinBox->value() / ratio)));
}

void MainWindow::UpdateColumnsFromRows()
{
    if (_image.isNull())
        return;

    if (!_ui.keepAspectRatioCheckBox->isChecked())
        return;

    auto ratio = _image.width() / static_cast<double>(_image.height());
    _ui.columnsSpinBox->setValue(static_cast<int>(std::round(_ui.rowsSpinBox->value() * ratio)));
}