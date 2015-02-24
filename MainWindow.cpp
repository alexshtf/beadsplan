#include "MainWindow.h"
#include "gco/GCoptimization.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/qmessagebox.h>

namespace
{
    double sqr(double x) { return x * x; }

    double ColorDiff(const QColor& left, const ColorCatalog::Entry& right)
    {
        auto dR = (left.red() - right.r);
        auto dG = (left.green() - right.g);
        auto dB = (left.blue() - right.b);

        return sqrt(sqr(dR) + sqr(dG) + sqr(dB)) / 255;
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

    GCoptimizationGridGraph gridGraph(
        static_cast<SiteID>(_image.width()),
        static_cast<SiteID>(_image.height()),
        static_cast<LabelID>(_colorCatalog.size())
    );

    gridGraph.setDataCost([] (SiteID s1, LabelID label, void* extra) {
        auto& image =  reinterpret_cast<MainWindow*>(extra)->_image;
        auto& catalog = reinterpret_cast<MainWindow*>(extra)->_colorCatalog;

        auto width = static_cast<SiteID>(image.width());
        auto x = s1 % width;
        auto y = (s1 - x) / width;

        auto imgColor = QColor::fromRgb(image.pixel(x, y));
        auto catalogColor = catalog.entryAt(static_cast<size_t>(label));

        return ColorDiff(imgColor, catalogColor);
    }, this);

    gridGraph.expansion();
}
