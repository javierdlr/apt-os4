#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <cctype>

#include "apt.h"
#include "aptcommand.h"
#include "stringutils.h"
#include "repositoryManager.h"

APT apt;

static void error(std::string error) {
    std::cerr << error << std::endl;
    exit(EXIT_FAILURE);
}

AptCommand parse_command(const std::string& cmd) {
    if (cmd == "install") return AptCommand::Install;
    if (cmd == "uninstall" || cmd == "remove") return AptCommand::Uninstall;
    if (cmd == "search") return AptCommand::Search;
    if (cmd == "update") return AptCommand::Update;
    return AptCommand::Unknown;
}

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  apt install <package1> <package2> ...\n";
    std::cout << "  apt uninstall <package1> <package2> ...\n";
    std::cout << "  apt remove <package1> <package2> ...\n";
    std::cout << "  apt search <package> ...\n";
    std::cout << "  apt update\n";
}


int main(int argc, char *argv[]) {
    int opt;
    std::string search_term;
    std::string package_name;
    std::string filename = APT_SOURCE_LIST;
    Logger logger(false);

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    AptCommand cmd = parse_command(command);

    if (cmd == AptCommand::Unknown) {
        logger.error("Unknown command: " + command);
        print_usage();
        return 1;
    }

    if (argc < 3) {
        logger.error("No packages specified.");
        print_usage();
        return 1;
    }

    // Long options structure
    static struct option long_options[] = {
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
    while ((opt = getopt_long(argc, argv, "v:p:h:", long_options, NULL)) != EOF) {
        switch (opt) {
            case 'v':
                apt.verbose(true);
                break;
            case 'p':
                apt.ignorePeers(true);
                break;
            case 'h':
                print_usage();
                return 0;
            default:
                error(APT_USAGE);
        }
    }

    logger.setEnabled(apt.verbose());

    RepositoryManager repositoryManager(apt.verbose());
    if (!repositoryManager.readRepositoryFile(filename)) {
        error("Cannot find " + filename + " file");
    }
    
    std::vector<std::string> packages;
    for (int i = 2; i < argc; ++i) {
        packages.push_back(argv[i]);
    }

    if (cmd == AptCommand::Update) {
        std::vector <Repository> repository = repositoryManager.downloadPackages();
        std::vector <Package> packagesToUpdate;
        // Check packages to update (if any)
        for (const auto &package: repositoryManager.packageList()) {
            if (repositoryManager.checkPackage(package)) {
                logger.log("Package " + package.name + " needs to be updated");
                packagesToUpdate.push_back(package);
            } else {
                logger.error("Package " + package.name + " not found or already updated");
            }
        }
        if (packagesToUpdate.size() > 0) {
            // Update packages
        }
    } else if (cmd == AptCommand::Install) {
        std::vector<Package> installedPackages = repositoryManager.installPackages(packages);
        for (const auto& p : installedPackages) {
            if (p.installed)
                logger.log("Package " + p.name + " installed");
            else
                logger.error("Package " + p.name + " not installed");
        }
    } else if (cmd == AptCommand::Search) {
        // Perform actions based on the specified parameters
        for (const auto& package : packages) {
            logger.debug("Searching for packages matching: " + package);
            // Add code for search action
            std::vector <Package> packagesFound = repositoryManager.searchPackages(StringUtils::tolower(package));
            for (const auto &p: packagesFound) {
                logger.log(p.name + "/" + p.version);
                logger.log("\t" + p.description);
            }
        }
    }
    else if (cmd == AptCommand::Uninstall) {
        std::string packagesToRemove = package_name.erase(package_name.find_last_not_of(" \n\r\t") + 1);
        logger.log("Removing '" + packagesToRemove + "'");
        // Add code for uninstall action
    }

    return 0;
}