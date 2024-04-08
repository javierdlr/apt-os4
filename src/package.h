#include <string>

// Package class to hold package information
class Package {
public:
    std::string name;
    std::string version;
    std::string architecture;
    std::string maintainer;
    std::vector<std::string> depends;
    std::vector<std::string> conflicts;
    std::string filename;
    unsigned int size;
    std::string md5sum;
    std::string sha1;
    std::string sha256;
    std::string section;
    std::string description;

    // Fields used to handle operations quickly
    std::string baseUrl;
};