
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
        RepositoryManager(bool verbose) : _verbose(verbose) { logger.setEnabled(verbose); }
        virtual bool readRepositoryFile(std::string path);
        virtual bool saveRepository(std::string name, std::string content);
        virtual std::vector<Repository> downloadPackages();
        virtual std::vector<Repository> repositoryList() const { return repositories; };
        virtual bool checkPackage(Package newPackage);
        virtual std::vector<Package> searchPackages(std::string package);
        virtual std::vector<Package> installPackages(std::vector<std::string> packages);
        virtual std::vector<Package> packageList() const { return packages; };
        virtual bool downloadFile(const std::string& url, const std::string& filename);
        bool verbose() { return _verbose; };
    private:
        Logger logger;
        std::vector<Repository> repositories;
        std::vector<Package> packages;
        bool _verbose = false;
        // Callback function to write data
        static size_t writeCallback(void* ptr, size_t size, size_t nmemb, std::stringstream* stream) {
            size_t bytes = size * nmemb;
            stream->write((const char*)ptr, bytes);
            return bytes;
        }
};