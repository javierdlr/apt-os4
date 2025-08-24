
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

#include <curl/curl.h>

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include "package.h"
#include "repository.h"
#include "stringutils.h"
#include "ar.h"
#include "apt.h"
#include "logger.h"

class RepositoryManager : public StringUtils {
    public:
        RepositoryManager(PackageDb& db, bool verbose, bool ignorePeers) : _db(db), _verbose(verbose), _ignorePeers(ignorePeers) { _logger.setVerbose(verbose); }
        ~RepositoryManager();
        bool readRepositoryFile(std::string path);
        bool saveRepository(std::string name, std::string content);
        std::vector<Repository> downloadPackages();
        std::vector<Repository> repositoryList() const { return _repositories; };
        bool checkPackage(Package newPackage);
        std::vector<Package> searchPackages(std::string package);
        std::vector<Package> installPackages(std::vector<std::string> packages);
        std::vector<Package> packageList() const { return _packages; };
        bool downloadFile(const std::string& url, const std::string& filename);
        void upgradeAllPackages();
        std::vector<Package> removePackages(std::vector<std::string> packages);

    private:
        Logger _logger;
        PackageDb& _db;
        std::vector<Repository> _repositories;
        std::vector<Package> _packages;
        bool _verbose = false;
        bool _ignorePeers = false;
        // Callback function to write data
        static size_t writeCallback(void* ptr, size_t size, size_t nmemb, std::stringstream* stream) {
            size_t bytes = size * nmemb;
            stream->write((const char*)ptr, bytes);
            return bytes;
        }
};