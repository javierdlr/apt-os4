#include "ar.h"

#include <cstring>

bool ArExtractor::copy_file(const fs::path& source, const fs::path& destination)
{
    if (!fs::exists(source)) {
        return false;
    }

    if (fs::exists(destination)) {
        fs::remove(destination);
    }

    char* buffer = (char *) malloc(BUFFER_SIZE);
    if (NULL == buffer) {
        return false;
    }

    int fd_from = open(source.c_str(), O_RDONLY);
    if (fd_from < 0) {
        free(buffer);
        return false;
    }
    int fd_to = open(destination.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd_to < 0) {
        free(buffer);
        // Avoid leaking a file handle in case of error.
        close(fd_from);
        return false;
    }

    // Infinite loop, exiting when nothing more can be read
    for(;;) {
        int ch_r;
        ch_r = read(fd_from, buffer, BUFFER_SIZE);
        if (ch_r) {
            if (ch_r != write(fd_to, buffer, ch_r)) {
                // Out of storage space?
                close(fd_from);
                close(fd_to);
                free(buffer);
                fs::remove(destination);
                return false;
            }
        } else {
            // finished
            break;
        }
    }
    close(fd_to);
    close(fd_from);
    free(buffer);
    return true;
}

bool ArExtractor::safe_rename(const fs::path& src, const fs::path& dest) {
    // Source must be a file
    if (!fs::exists(src) || !fs::is_regular_file(src)) {
        return false;
    }

    // If destination exists, it must not be a directory
    if (fs::exists(dest) && fs::is_directory(dest)) {
        return false;
    }

    // Ensure parent directories for destination
    fs::create_directories(dest.parent_path());

    // Perform rename
    if (copy_file(src, dest)) {
		fs::remove(src);
        return true;
    }
	else {
		_logger.error("Failed to move " + src.string() + " -> " + dest.string());
	}
    return false;
}

// Remove all instances of "//./" from a path
std::string ArExtractor::clean_path(const std::string& path) {
    std::string result = path;
    const std::string artifact = "//./";
    size_t pos;
    while ((pos = result.find(artifact)) != std::string::npos) {
        result.replace(pos, artifact.length(), "/");
    }
    return result;
}

// Helper: transform extracted path to Amiga SDK format
std::string ArExtractor::amiga_sdk_path(const std::string& extracted) {
	// Replace //.
	// Find "/SDK/local/" and "/SDK/"
    size_t sdk_pos = extracted.find("/SDK/local/");
    if (sdk_pos == std::string::npos) {
        sdk_pos = extracted.find("/SDK/");
        if (sdk_pos == std::string::npos) {
            return "";
        }
    }

    // Start from "SDK/local/" or "SDK/"
    std::string subpath = extracted.substr(sdk_pos + 1); // skip the first '/'
    // Replace first '/' with ':'
    size_t colon_pos = subpath.find('/');
    if (colon_pos != std::string::npos) {
        subpath.replace(colon_pos, 1, ":");
    }
    return subpath;
}

// Helper to extract a tarball (data.tar.gz or data.tar.xz) from memory
int ArExtractor::extract_tarball(const void *buffer, size_t size, const char *target_dir, std::vector<std::string>& extracted_files) {
    struct archive *a = archive_read_new();
    struct archive *ext = archive_write_disk_new();
    archive_read_support_format_empty(a);
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);
    archive_read_support_filter_xz(a);
    archive_read_support_filter_zstd(a);

    // Set disk writer options (extract permissions, times, etc)
    archive_write_disk_set_options(ext,
        ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
        ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS |
        ARCHIVE_EXTRACT_SECURE_NODOTDOT);

    if (archive_read_open_memory(a, buffer, size) != ARCHIVE_OK) {
        _logger.error("Failed to open tarball from memory: " + std::string(archive_error_string(a)));
        archive_read_free(a);
        archive_write_free(ext);
        return 1;
    }

    struct archive_entry *entry;
    int r;
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *orig_path = archive_entry_pathname(entry);
        char new_path[4096] = {0};
        snprintf(new_path, sizeof(new_path), "%s/%s", target_dir, orig_path);
        archive_entry_set_pathname(entry, new_path);

        archive_write_header(ext, entry);
        const void *buff;
        size_t size_buff;
        int64_t offset;
        while (archive_read_data_block(a, &buff, &size_buff, &offset) == ARCHIVE_OK) {
            archive_write_data_block(ext, buff, size_buff, offset);
        }
        archive_write_finish_entry(ext);
		// Move file to Amiga SDK path
		std::string amiga_path = amiga_sdk_path(new_path);
		if (!amiga_path.empty()) {
			std::string cleaned_path = clean_path(new_path);
            if (fs::is_regular_file(cleaned_path)) {
				if (!safe_rename(cleaned_path.c_str(), amiga_path.c_str())) {
					_logger.error("Failed to move " + cleaned_path + " to " + amiga_path);
				}
				else {
					extracted_files.push_back(amiga_path);
				}
			}
		}
    }

    archive_read_free(a);
    archive_write_free(ext);
    return 0;
}

bool ArExtractor::extract(const std::string& filename, const std::string& outputDir, std::vector<std::string>& extracted_files) {
	struct archive *a = archive_read_new();
    archive_read_support_format_ar(a);
    if (archive_read_open_filename(a, filename.c_str(), 10240) != ARCHIVE_OK) {
        _logger.error("Could not open deb file: " + std::string(archive_error_string(a)));
        archive_read_free(a);
        return false;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *name = archive_entry_pathname(entry);

        // Look for data.tar.gz or data.tar.xz
        if (strstr(name, "data.tar") == name) {
            size_t size = archive_entry_size(entry);
            void *buffer = malloc(size);
            if (!buffer) {
                _logger.error("Memory allocation failed");
                archive_read_free(a);
                return false;
            }
            archive_read_data(a, buffer, size);

            // Extract the embedded tarball
            extract_tarball(buffer, size, outputDir.c_str(), extracted_files);
            free(buffer);
        } else {
            // Skip other entries
            archive_read_data_skip(a);
        }
    }

    archive_read_free(a);
    return true;
}
