#pragma once

#include <string>
#include <vector>

class ColorCatalog
{
public:
    struct Entry
    {
        std::string name;
        int r, g, b;

        Entry() = default;
        Entry(const Entry&) = default;
        Entry(std::string name, int r, int g, int b);
    };

    ColorCatalog();

    size_t size() { return _entries.size(); }
    Entry& entryAt(size_t index) { return _entries[index]; }

private:

    std::vector<Entry> _entries;
};

