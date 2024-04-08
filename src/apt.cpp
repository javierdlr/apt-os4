#include "apt.h"

bool APT::createDirs() {
    struct stat st;
    // Check for directory structure
    if (stat(APT_ASSIGN, &st) != 0) {
        std::cerr << "APT assign is not present!" << std::endl;
        return false;
    }

    if (stat(APT_LISTS, &st) != 0) {
        if (mkdir(APT_LISTS, 0700) != 0) {
            std::cerr << "Cannot create lists directory!" << std::endl;
            return false;
        }
    }

    if (stat(APT_PACKAGES, &st) != 0) {
        if (mkdir(APT_PACKAGES, 0700) != 0) {
            std::cerr << "Cannot create packages directory!" << std::endl;
            return false;
        }
    }

    if (stat(APT_TEMPDIR, &st) != 0) {
        if (mkdir(APT_TEMPDIR, 0700) != 0) {
            std::cerr << "Cannot create temporary directory!" << std::endl;
            return false;
        }
    }

    return true;
}