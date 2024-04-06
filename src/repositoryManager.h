
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <curl/curl.h>

#include "package.h"
#include "repository.h"
#include "apt.h"

class RepositoryManager {
    public:
        virtual bool readRepositoryFile(std::string path);
        virtual std::vector<Repository> downloadPackages();
        virtual bool saveRepository(std::string name, std::string content);
        virtual std::vector<Package> packageList() const { return packages; };
        virtual std::vector<Repository> repositoryList() const { return repositories; };

    private:
        std::vector<Repository> repositories;
        std::vector<Package> packages;
        // Callback function to write data
        static size_t writeCallback(void* ptr, size_t size, size_t nmemb, std::stringstream* stream) {
            size_t bytes = size * nmemb;
            stream->write((const char*)ptr, bytes);
            return bytes;
        }
        static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
            std::string retval = str;
            size_t start_pos = retval.find(from);
            if(start_pos == std::string::npos)
                return str;
            retval.replace(start_pos, from.length(), to);
            return retval;
        }
        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
            std::string retval = str;
            if (from.empty())
                return str;
            size_t start_pos = 0;
            while((start_pos = retval.find(from, start_pos)) != std::string::npos) {
                retval.replace(start_pos, from.length(), to);
                start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
            }
            return retval;
        }
};