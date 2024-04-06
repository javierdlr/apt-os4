#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <sys/stat.h>

#include "apt.h"
#include "repositoryManager.h"

APT apt;

static void error(std::string error) {
    std::cerr << error << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int opt;
    std::string search_term;
    std::string package_name;
    int install_flag = 0;
    int remove_flag = 0;
    int search_flag = 0;
    int update_flag = 0;
    RepositoryManager repository;
    struct stat st;

    // Long options structure
    static struct option long_options[] = {
        {"search", required_argument, 0, 's'},
        {"install", required_argument, 0, 'i'},
        {"remove", required_argument, 0, 'r'},
        {"update", no_argument, 0, 'u'},
        {"ignorepeers", no_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    apt.verbose(false);
    apt.ignorePeers(false);

    // Check for directory structure
    if (stat(APT_ASSIGN, &st) != 0) {
        error("APT assign is not present!\n");
        exit(EXIT_FAILURE);
    }

    if (stat(APT_LISTS, &st) != 0) {
        if (mkdir(APT_LISTS, 0700) != 0) {
            error("Cannot create lists directory!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (stat(APT_PACKAGES, &st) != 0) {
        if (mkdir(APT_PACKAGES, 0700) != 0) {
            error("Cannot create packages directory!\n");
            exit(EXIT_FAILURE);
        }
    }

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "s:i:r:u:v:p:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                search_term = optarg;
                if (search_term.empty()) {
                    error("Search: <package> is required");
                }
                search_flag = 1;
                break;
            case 'i':
                package_name = optarg;
                if (package_name.empty()) {
                    error("Install: <package> is required");
                }
                install_flag = 1;
                break;
            case 'r':
                package_name = optarg;
                if (package_name.empty()) {
                    error("Remove: <package> is required");
                }
                remove_flag = 1;
                break;
            case 'u':
                update_flag = 1;
                break;
            case 'v':
                apt.verbose(true);
                break;       
            case 'p':
                apt.ignorePeers(true);
                break;       
            default:
                error(APT_USAGE);
        }
    }
    if (!install_flag && !remove_flag && !search_flag && !update_flag) {
        error(APT_USAGE);
    }

    // Ensure one and only one option is provided
    if (install_flag + remove_flag + search_flag + update_flag > 1) {
        error("Only one option can be provided.");
    }

    std::string filename = "sources.list";
    if (!repository.readRepositoryFile(filename)) {
        exit(EXIT_FAILURE);
    }

    if (update_flag) {
        std::vector<Repository> package = repository.downloadPackages();
        // Display downloaded package information
        for (const auto& package : repository.packageList()) {
            /*
            std::cout << "Name: " << package.name << std::endl;
            std::cout << "Version: " << package.version << std::endl;
            std::cout << "Architecture: " << package.architecture << std::endl;
            std::cout << "Maintainer: " << package.maintainer << std::endl;
            std::cout << "Depends: ";
            for (const auto& depend : package.depends) {
                std::cout << depend << ", ";
            }
            std::cout << std::endl;
            std::cout << "Filename: " << package.filename << std::endl;
            std::cout << "Size: " << package.size << std::endl;
            std::cout << "MD5sum: " << package.md5sum << std::endl;
            std::cout << "SHA1: " << package.sha1 << std::endl;
            std::cout << "SHA256: " << package.sha256 << std::endl;
            std::cout << "Section: " << package.section << std::endl;
            std::cout << "Description: " << package.description << std::endl;
            std::cout << std::endl;
            */
        }
    }
    else {
        // Perform actions based on the specified parameters
        if (!search_term.empty()) {
            std::cout << "Performing search action with term: " << search_term << "\n";
            // Add code for search action
        }
        if (!package_name.empty()) {
            std::cout << "Performing install action for package: " << package_name << "\n";
            // Add code for install action
        }
    }

    return 0;
}