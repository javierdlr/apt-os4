#include <string>
#include <iostream>

#include <archive.h>
#include <archive_entry.h>

class ArExtractor {
    public:
        bool extract(const std::string& filename, const std::string& outputDir);
};