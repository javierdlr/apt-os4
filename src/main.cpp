#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <cctype>

#include "apt.h"
#include "stringutils.h"
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
    RepositoryManager repositoryManager;
    std::string filename = APT_SOURCE_LIST;

    // Long options structure
    static struct option long_options[] = {
            {"search",      required_argument, 0, 's'},
            {"install",     required_argument, 0, 'i'},
            {"remove",      required_argument, 0, 'r'},
            {"update",      no_argument,       0, 'u'},
            {"ignorepeers", no_argument,       0, 'p'},
            {"verbose",     no_argument,       0, 'v'},
            {"help",        no_argument,       0, 'h'},
            {0,             0,                 0, 0}
    };

    apt.verbose(false);
    apt.ignorePeers(false);

    if (!apt.createDirs()) {
        exit(EXIT_FAILURE);
    }

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "s:i:r:u:v:p:", long_options, NULL)) != EOF) {
        switch (opt) {
            case 's':
                search_term = optarg;
                if (search_term.empty()) {
                    error("Search: <package> is required");
                }
                search_flag = 1;
                break;
            case 'i':
                if (!package_name.empty())
                    package_name += " ";
                package_name += optarg;
                if (package_name.empty()) {
                    error("Install: <package> is required");
                }
                install_flag = 1;
                break;
            case 'r':
                if (!package_name.empty())
                    package_name += " ";
                package_name += optarg;
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
            case 'h':
                error(APT_USAGE);
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

    if (!repositoryManager.readRepositoryFile(filename)) {
        error("Cannot find " + filename + " file");
    }

    if (update_flag) {
        std::vector <Repository> repository = repositoryManager.downloadPackages();
        std::vector <Package> packagesToUpdate;
        // Check packages to update (if any)
        for (const auto &package: repositoryManager.packageList()) {
            if (repositoryManager.checkPackage(package)) {
                std::cout << "Package " << package.name << " needs to be updated" << std::endl;
                packagesToUpdate.push_back(package);
            } else {
                std::cout << "Package " << package.name << " not found or already updated" << std::endl;
            }
        }
        if (packagesToUpdate.size() > 0) {
            // Update packages
        }
    } else if (install_flag) {
        std::string packagesToInstall = package_name.erase(package_name.find_last_not_of(" \n\r\t") + 1);
        std::cout << "Installing '" << packagesToInstall << "'" << std::endl;
        repositoryManager.installPackages(packagesToInstall);
    } else if (search_flag) {
        // Perform actions based on the specified parameters
        if (!search_term.empty()) {
            std::cout << "Performing search action with term: " << search_term << "\n";
            // Add code for search action
            std::vector <Package> packagesFound = repositoryManager.searchPackages(StringUtils::tolower(search_term));
            for (const auto &p: packagesFound) {
                std::cout << p.name << "/" << p.version << std::endl << "\t" << p.description << std::endl;
            }
        }
    }

    return 0;
}