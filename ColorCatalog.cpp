#include "ColorCatalog.h"
#include <QtGui/QColor>

namespace {

    std::vector<ColorCatalog::Entry> GetQtColors()
    {
        std::vector<ColorCatalog::Entry> result;
        for(auto name : QColor::colorNames())
        {
            auto color = QColor(name).toRgb();
            result.emplace_back(name.toStdString(), color.red(), color.green(), color.blue());
        }

        return result;
    }

}

ColorCatalog::Entry::Entry(std::string name, int r, int g, int b)
    : name(std::move(name))
      , r(r)
      , g(g)
      , b(b)
{}

ColorCatalog::ColorCatalog()
    : _entries(GetQtColors())
{}