#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <cctype>

#include "apt.h"
#include "aptcommand.h"
#include "stringutils.h"
#include "repositoryManager.h"
#include "package.h"

static const char __attribute__((used)) *_version = "\0$VER: apt for clib4 repository. Version " VERSION " (" __DATE__ ")";


static void error(std::string error) {
    std::cerr << error << std::endl;
    exit(EXIT_FAILURE);
}

AptCommand parse_command(const std::string& cmd) {
    if (cmd == "install") return AptCommand::Install;
    if (cmd == "uninstall" || cmd == "remove") return AptCommand::Uninstall;
    if (cmd == "search") return AptCommand::Search;
    if (cmd == "update") return AptCommand::Update;
    if (cmd == "upgrade") return AptCommand::Upgrade;
    if (cmd == "list") return AptCommand::List;
    return AptCommand::Unknown;
}

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  apt [--verbose] [--ignorepeers] install <package1> <package2> ...\n";
    std::cout << "  apt [--verbose] [--ignorepeers] uninstall <package1> <package2> ...\n";
    std::cout << "  apt [--verbose] [--ignorepeers] remove <package1> <package2> ...\n";
    std::cout << "  apt [--verbose] [--ignorepeers] search <package> ...\n";
    std::cout << "  apt [--verbose] [--ignorepeers] update\n";
    std::cout << "  apt [--verbose] [--ignorepeers] upgrade\n";
    std::cout << "  apt [--verbose] [--ignorepeers] list\n";
}


int main(int argc, char *argv[]) {
    int opt;
    int optional_opts = 0;
    std::string search_term;
    std::string package_name;
    std::string filename = APT_SOURCE_LIST;
    Logger logger(false);
    APT apt;

    // Long options structure
    static struct option long_options[] = {
            {"ignorepeers", no_argument,       0, 'p'},
            {"verbose",     no_argument,       0, 'v'},
            {"help",        no_argument,       0, 'h'},
            {0,             0,                 0, 0}
    };

    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "ab:c:", long_options, NULL)) != EOF) {
        switch (opt) {
            case 'p':
                optional_opts++;
                apt.ignorePeers(true);
                break;
            case 'v':
                optional_opts++;
                apt.verbose(true);
                break;
            case 'h':
            default:
                print_usage();
                return 0;
        }
    }

    std::string command = argv[optional_opts + 1];
    AptCommand cmd = parse_command(command);

    if (cmd == AptCommand::Unknown) {
        logger.error("Unknown command: " + command);
        print_usage();
        return EXIT_FAILURE;
    }

    if ((cmd != AptCommand::Update && cmd != AptCommand::Upgrade && cmd != AptCommand::List) && argc < 3 + optional_opts) {
        logger.error("No packages specified.");
        print_usage();
        return EXIT_FAILURE;
    }

    logger.setVerbose(apt.verbose());

    if (!apt.createDirs()) {
        exit(EXIT_FAILURE);
    }

    PackageDb db(APT_PACKAGES"packages.db");
    if (!db.open()) {
        std::cerr << "Could not open DB\n";
        return EXIT_FAILURE;
    }
    if (!db.createTables()) {
        std::cerr << "Could not create tables\n";
        return EXIT_FAILURE;
    }

    RepositoryManager repositoryManager(db, apt.verbose(), apt.ignorePeers());
    if (!repositoryManager.readRepositoryFile(filename)) {
        error("Cannot find " + filename + " file");
    }
    
    std::vector<std::string> packages;
    for (int i = 2 + optional_opts; i < argc; ++i) {
        packages.push_back(argv[i]);
    }

    if (cmd == AptCommand::List) {
        std::vector<Package> installedPackages = db.queryPackages();
        size_t i = 1;
        for (const auto& p : installedPackages) {
            logger.log(p.name + "/" + p.version + (p.installed ? " (installed)" : "") + (p.needs_update ? " *" : ""));
            logger.log("\t" + p.description);
            if (i < installedPackages.size()) {
                logger.log("");
            }
            i++;
        }
        return 0;
    }
    if (cmd == AptCommand::Update) {
        // Download new packages from repository
        std::vector <Repository> repository = repositoryManager.downloadPackages();
        std::vector <Package> packagesToUpdate;
        // Check packages to update (if any)
        for (const auto &package: repositoryManager.packageList()) {
            if (repositoryManager.checkPackage(package)) {
                logger.log("Package " + package.name + " needs to be updated");
                packagesToUpdate.push_back(package);
            } else {
                logger.debug("Package " + package.name + " not found or already updated");
            }
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
        for (const auto& package : packages) {
            std::vector <Package> packagesFound = repositoryManager.searchPackages(StringUtils::tolower(package));
            size_t i = 1;
            for (const auto &p: packagesFound) {
                logger.log(p.name + "/" + p.version + (p.installed ? " (installed)" : "") + (p.needs_update ? " *" : ""));
                logger.log("\t" + p.description);
                if (i < packagesFound.size()) {
                    logger.log("");
                }
                i++;
            }
        }
    }
    else if (cmd == AptCommand::Uninstall) {
        std::vector<Package> removedPackages = repositoryManager.removePackages(packages);
        for (const auto& p : removedPackages) {
            logger.log("Package " + p.name + " removed");
        }
    }
    else if (cmd == AptCommand::Upgrade) {
        repositoryManager.upgradeAllPackages();
    }

    return 0;
}