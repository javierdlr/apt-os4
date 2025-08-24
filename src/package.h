#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>

#include <sqlite3.h>

#include "logger.h"

// Package class to hold package information
class Package {
public:
    int id = 0;
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

    // Field used to check if it is installed or not
    bool installed = false;
    bool needs_update = false;

    // Foreign key to repository
    int repository_id = 0;
};

struct RepositoryInfo {
    int id = 0;
    std::string name;
    std::string url;
};

struct PackageFile {
    int id = 0;
    int package_id = 0;           // Foreign key to packages table
    std::string filepath;
};

struct PackageUpdate {
    int id = 0;
    int package_id = 0;           // Foreign key to packages table
    std::string package_name;
    std::string old_version;
    std::string new_version;
    std::string update_time; // ISO8601 timestamp
};

class PackageDb {
public:
    PackageDb(const std::string& dbfile, bool verbose = false) : _dbfile(dbfile), _db(nullptr) { _logger.setVerbose(verbose); }
    ~PackageDb();

    // DB Handling
    bool open();
    void close();
    bool createTables();

    // Package Handling
    bool insertPackage(const Package& pkg);
    bool insertOrUpdatePackage(const Package& pkg);
    std::vector<Package> queryPackages(const std::string& name = "");
    bool setInstalled(int id, bool installed);
    int insertRepository(const RepositoryInfo& repo);
    std::vector<RepositoryInfo> queryRepositories();
    RepositoryInfo queryRepository(int id);

    // Package file handling
    bool insertPackageFile(int package_id, const std::string& filepath);
    bool insertPackageFiles(int package_id, const std::vector<std::string>& filepaths);
    std::vector<PackageFile> queryPackageFiles(int package_id);
    bool removePackageFiles(int package_id);
    std::vector<Package> queryPackagesNeedingUpdate();
    bool deletePackageFiles(int package_id);
    bool setPackageNeedsUpdate(int package_id, bool needs_update);
    bool insertPackageUpdate(const PackageUpdate& update);
    bool deletePackageUpdate(int update_id);
    std::vector<PackageUpdate> queryPackageUpdates(int package_id);
private:
    std::string _dbfile;
    sqlite3* _db;
    Logger _logger;

    bool isNewerVersion(const std::string& newVersion, const std::string& oldVersion);
    std::vector<int> splitVersion(const std::string& version);
    std::string getCurrentTimestamp();
    bool execSQL(const char* sql, const std::string& context);
};

#endif // PACKAGE_H