#include "ar.h"

bool ArExtractor::extract(const std::string& filename, const std::string& outputDir) {
    struct archive* a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    int r = archive_read_open_filename(a, filename.c_str(), 10240); // 10240 is the block size
    if (r != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a);
        archive_read_free(a);
        return -1;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string entryPath = outputDir + "/" + archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, entryPath.c_str());
        archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME);
    }

    archive_read_close(a);
    archive_read_free(a);
    return 0;
}