#include "MainWindow.h"
#include "gco/GCoptimization.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/qmessagebox.h>
#include <memory>

namespace
{
    double sqr(double x) { return x * x; }

    double ColorDiff(const QColor& left, const ColorCatalog::Entry& right)
    {
        auto dR = (left.red() - right.r);
        auto dG = (left.green() - right.g);
        auto dB = (left.blue() - right.b);

        return sqrt(sqr(dR) + sqr(dG) + sqr(dB)) / 16;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    connect(_ui.openImageButton, &QPushButton::pressed, [&] { LoadImage(); });
    connect(_ui.generatePlanButton, &QPushButton::pressed, [&] { GeneratePlan(); });
}

void MainWindow::LoadImage()
{
    QString imageFile = QFileDialog::getOpenFileName(this, tr("Open image"), QString(), tr("Images (*.png *.jpg *.xpm);;All files (*.*)"));
    if (!imageFile.isEmpty())
    {
        _image = QImage();
        if (_image.load(imageFile))
            _ui.imageLabel->setPixmap(QPixmap::fromImage(_image));
        else
            QMessageBox::warning(this, tr("Cannot open image"), imageFile);
    }
}

void MainWindow::GeneratePlan()
{
    using SiteID = GCoptimization::SiteID;
    using LabelID = GCoptimization::LabelID ;

    try
    {
        GCoptimizationGridGraph gridGraph(
            static_cast<SiteID>(_image.width()),
            static_cast<SiteID>(_image.height()),
            static_cast<LabelID>(_colorCatalog.size())
        );

        std::unique_ptr<double[]> dataCost(new double[gridGraph.numSites() * gridGraph.numLabels()]);
        for(SiteID x = 0; x < _image.width(); ++x)
        {
            for(SiteID y = 0; y < _image.height(); ++y)
            {
                auto imgColor = QColor::fromRgb(_image.pixel(x, y));
                auto siteId = y * _image.width() + x;
                for(LabelID l = 0; l < gridGraph.numLabels(); ++l)
                {
                    auto& catalogColor = _colorCatalog.entryAt(static_cast<size_t>(l));
                    dataCost[siteId * gridGraph.numLabels() + l] = ColorDiff(imgColor, catalogColor);
                }
            }
        }

        gridGraph.setDataCost(dataCost.get());
        gridGraph.expansion();

        for(SiteID x = 0; x < _image.width(); ++x)
        {
            for(SiteID y = 0; y < _image.height(); ++y)
            {
                auto id = y * _image.width() + x;
                auto label = gridGraph.whatLabel(id);
                auto color = _colorCatalog.entryAt(static_cast<size_t>(label));
                _image.setPixel(x, y, qRgb(color.r, color.g, color.b));
            }
        }
        _ui.imageLabel->setPixmap(QPixmap::fromImage(_image));
    }
    catch(const GCException& e)
    {
        QMessageBox::warning(this, tr("Unable to compute plan"), e.message);
    }
}
