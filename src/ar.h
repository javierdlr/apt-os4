#include <string>
#include <iostream>

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>

#include "logger.h"
#include "package.h"

namespace fs = std::filesystem;

#define BUFFER_SIZE 1048576

class ArExtractor {
    public:
        bool extract(const std::string& filename, const std::string& outputDir, std::vector<std::string>& extracted_files);
    private:
        Logger _logger;

        int extract_tarball(const void *buffer, size_t size, const char *target_dir, std::vector<std::string>& extracted_files);
        std::string amiga_sdk_path(const std::string& extracted);
        std::string clean_path(const std::string& path);
        bool safe_rename(const fs::path& src, const fs::path& dest);
        bool copy_file(const fs::path& source, const fs::path& destination);
};