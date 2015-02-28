#pragma once

#include <string>
#include <bits/stl_bvector.h>

class ColorCatalog
{
public:
    struct Entry
    {
        std::string name;
        int r, g, b;

        Entry() {}
        Entry(std::string name, int r, int g, int b)
            : name(std::move(name))
            , r(r)
            , g(g)
            , b(b)
        {}
    };

    ColorCatalog()
        : _entries { {"red", 255, 0, 0}, {"green", 0, 255, 0}, {"blue", 0, 0, 255} }
    {}

    size_t size() { return _entries.size(); }
    Entry& entryAt(size_t index) { return _entries[index]; }

private:
    std::vector<Entry> _entries;
};