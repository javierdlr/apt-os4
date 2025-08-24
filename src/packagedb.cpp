#include <ctime>
#include <sstream>
#include <iostream>

#include "package.h"

PackageDb::~PackageDb() { close(); }

bool PackageDb::open() {
    return sqlite3_open(_dbfile.c_str(), &_db) == SQLITE_OK;
}

void PackageDb::close() {
    if (_db) {
        sqlite3_close(_db);
        _db = nullptr;
    }
}

bool PackageDb::execSQL(const char* sql, const std::string& context) {
    char* errmsg = nullptr;
    int rc = sqlite3_exec(_db, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        _logger.error("SQL error in " + context + ": " + std::string(errmsg));
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool PackageDb::createTables() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS packages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "version TEXT, "
        "architecture TEXT, "
        "maintainer TEXT, "
        "depends TEXT, "
        "conflicts TEXT, "
        "filename TEXT, "
        "size INTEGER, "
        "md5sum TEXT, "
        "sha1 TEXT, "
        "sha256 TEXT, "
        "section TEXT, "
        "description TEXT, "
        "installed INTEGER DEFAULT 0, "
        "needs_update INTEGER DEFAULT 0, "
        "repository_id INTEGER, "
        "FOREIGN KEY (repository_id) REFERENCES repositories (id));";
    if (!execSQL(sql, "packages")) return false;

    const char* sql_repos =
        "CREATE TABLE IF NOT EXISTS repositories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT UNIQUE NOT NULL, "
        "url TEXT"
        ");";
    if (!execSQL(sql_repos, "repositories")) return false;

    const char* sql_updates =
        "CREATE TABLE IF NOT EXISTS package_updates ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "package_id INTEGER, "
        "package_name TEXT, "
        "old_version TEXT, "
        "new_version TEXT, "
        "update_time TEXT, "
        "FOREIGN KEY (package_id) REFERENCES packages (id));";
    if (!execSQL(sql_updates, "package_updates")) return false;

    const char* sql_files =
        "CREATE TABLE IF NOT EXISTS package_files ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "package_id INTEGER, "
        "filepath TEXT, "
        "FOREIGN KEY(package_id) REFERENCES packages(id)"
        ");";
    if (!execSQL(sql_files, "package_files")) return false;

    return true;
}

bool PackageDb::insertPackage(const Package& pkg) {
    std::stringstream depends_ss, conflicts_ss;
    for (size_t i = 0; i < pkg.depends.size(); ++i) {
        if (i > 0) depends_ss << ",";
        depends_ss << pkg.depends[i];
    }
    for (size_t i = 0; i < pkg.conflicts.size(); ++i) {
        if (i > 0) conflicts_ss << ",";
        conflicts_ss << pkg.conflicts[i];
    }

 // Insert new package
    sqlite3_stmt* stmt;
    const char* sql_insert =
        "INSERT INTO packages (name, version, architecture, maintainer, depends, conflicts, filename, size, md5sum, sha1, sha256, section, description, installed, repository_id, needs_update) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(_db, sql_insert, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }
    sqlite3_bind_text(stmt, 1, pkg.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pkg.version.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, pkg.architecture.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, pkg.maintainer.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, depends_ss.str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, conflicts_ss.str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, pkg.filename.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, pkg.size);
    sqlite3_bind_text(stmt, 9, pkg.md5sum.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, pkg.sha1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, pkg.sha256.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, pkg.section.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 13, pkg.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 14, pkg.installed ? 1 : 0);
    sqlite3_bind_int(stmt, 15, pkg.repository_id);
    sqlite3_bind_int(stmt, 16, 0);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        _logger.error("Insert failed: " + std::string(sqlite3_errmsg(_db)));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

std::vector<int> PackageDb::splitVersion(const std::string& version) {
    std::vector<int> numbers;
    std::istringstream iss(version);
    std::string part;
    while (std::getline(iss, part, '.')) {
        numbers.push_back(std::stoi(part));
    }
    return numbers;
}

// Compare two version strings (returns true if v1 > v2)
bool PackageDb::isNewerVersion(const std::string& v1, const std::string& v2) {
    std::vector<int> nums1 = splitVersion(v1);
    std::vector<int> nums2 = splitVersion(v2);

    // Pad shorter version with zeros
    size_t maxlen = std::max(nums1.size(), nums2.size());
    nums1.resize(maxlen, 0);
    nums2.resize(maxlen, 0);

    for (size_t i = 0; i < maxlen; ++i) {
        if (nums1[i] > nums2[i]) return true;
        if (nums1[i] < nums2[i]) return false;
    }
    return false; // versions are equal
}

// Insert or update package if new version
bool PackageDb::insertOrUpdatePackage(const Package& pkg) {
    // Check for existing package by name
    const char* sql_check = "SELECT id, version, installed FROM packages WHERE name = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql_check, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }
    sqlite3_bind_text(stmt, 1, pkg.name.c_str(), -1, SQLITE_TRANSIENT);

    int pkg_id = 0;
    int installed = 0;
    std::string db_version;
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        pkg_id = sqlite3_column_int(stmt, 0);
        db_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        installed = sqlite3_column_int(stmt, 2) != 0;
        exists = true;
    }
    sqlite3_finalize(stmt);

    if (exists) {
        if (isNewerVersion(pkg.version, db_version)) {
            bool ok = true;
            _logger.log("Package " + pkg.name + " has a newer version " + pkg.version);
            if (installed) {
                ok = insertPackageUpdate(PackageUpdate{
                    .package_id = pkg_id,
                    .package_name = pkg.name,
                    .old_version = db_version,
                    .new_version = pkg.version,
                    .update_time = getCurrentTimestamp()
                });
            }

            if (ok) {
                // Update package
                const char* sql_update =
                    "UPDATE packages SET version=?, architecture=?, maintainer=?, depends=?, conflicts=?, filename=?, size=?, md5sum=?, sha1=?, sha256=?, section=?, description=?, installed=?, repository_id=?, needs_update=? WHERE id=?;";
                if (sqlite3_prepare_v2(_db, sql_update, -1, &stmt, nullptr) != SQLITE_OK) {
                    _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
                    return false;
                }

                std::stringstream depends_ss, conflicts_ss;
                for (size_t i = 0; i < pkg.depends.size(); ++i) {
                    if (i > 0) depends_ss << ",";
                    depends_ss << pkg.depends[i];
                }
                for (size_t i = 0; i < pkg.conflicts.size(); ++i) {
                    if (i > 0) conflicts_ss << ",";
                    conflicts_ss << pkg.conflicts[i];
                }

                sqlite3_bind_text(stmt, 1, pkg.version.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, pkg.architecture.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, pkg.maintainer.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, depends_ss.str().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 5, conflicts_ss.str().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 6, pkg.filename.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 7, pkg.size);
                sqlite3_bind_text(stmt, 8, pkg.md5sum.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 9, pkg.sha1.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 10, pkg.sha256.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 11, pkg.section.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 12, pkg.description.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 13, installed ? 1 : 0);
                sqlite3_bind_int(stmt, 14, pkg.repository_id);
                sqlite3_bind_int(stmt, 15, installed ? 1 : 0);
                sqlite3_bind_int(stmt, 16, pkg_id);

                int rc = sqlite3_step(stmt);
                bool ok = rc == SQLITE_DONE;
                if (!ok) 
                    _logger.error("Update failed: " + std::string(sqlite3_errmsg(_db)));
                sqlite3_finalize(stmt);

                return ok;
            } else {
                _logger.error("Failed to insert package update record for " + pkg.name);
                return false;
            }
        } else {
            // No update needed
            return true;
        }
    } else {
        // Insert as new package (use your previous insert code here)
        return insertPackage(pkg);
    }
}

std::vector<Package> PackageDb::queryPackages(const std::string& name) {
    std::vector<Package> result;
    std::string sql = "SELECT id, name, version, architecture, maintainer, depends, conflicts, filename, size, md5sum, sha1, sha256, section, description, installed, repository_id, needs_update FROM packages";
    sqlite3_stmt* stmt = nullptr;

    if (!name.empty()) {
        sql += " WHERE name LIKE ?";
        if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
            return result;
        }
        std::string like_pattern = "%" + name + "%";
        sqlite3_bind_text(stmt, 1, like_pattern.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
            return result;
        }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Package pkg;
        pkg.id = sqlite3_column_int(stmt, 0);
        pkg.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        pkg.version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        pkg.architecture = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        pkg.maintainer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string dependsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string conflictsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        pkg.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        pkg.size = sqlite3_column_int(stmt, 8);
        pkg.md5sum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        pkg.sha1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        pkg.sha256 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        pkg.section = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        pkg.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        pkg.installed = sqlite3_column_int(stmt, 14) != 0;
        pkg.repository_id = sqlite3_column_int(stmt, 15);
        pkg.needs_update = sqlite3_column_int(stmt, 16) != 0;

        RepositoryInfo repo = queryRepository(pkg.repository_id);
        pkg.baseUrl = repo.url;

        // Split depends/conflicts
        size_t start = 0, end;
        while ((end = dependsStr.find(',', start)) != std::string::npos) {
            pkg.depends.push_back(dependsStr.substr(start, end - start));
            start = end + 1;
        }
        if (start < dependsStr.size()) pkg.depends.push_back(dependsStr.substr(start));
        start = 0;
        while ((end = conflictsStr.find(',', start)) != std::string::npos) {
            pkg.conflicts.push_back(conflictsStr.substr(start, end - start));
            start = end + 1;
        }
        if (start < conflictsStr.size()) pkg.conflicts.push_back(conflictsStr.substr(start));
        result.push_back(pkg);
    }
    sqlite3_finalize(stmt);
    return result;
}

int PackageDb::insertRepository(const RepositoryInfo& repo) {
    // Check if repo exists by name
    const char* sql_check = "SELECT id FROM repositories WHERE name = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql_check, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, repo.name.c_str(), -1, SQLITE_TRANSIENT);

    int repo_id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        repo_id = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return repo_id;
    }
    sqlite3_finalize(stmt);

    // Not found, insert it
    const char* sql_insert = "INSERT INTO repositories (name, url) VALUES (?, ?);";
    if (sqlite3_prepare_v2(_db, sql_insert, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, repo.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, repo.url.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        repo_id = static_cast<int>(sqlite3_last_insert_rowid(_db));
    }
    sqlite3_finalize(stmt);
    return repo_id;
}

bool PackageDb::setInstalled(int id, bool installed) {
    const char* sql = "UPDATE packages SET installed = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }

    sqlite3_bind_int(stmt, 1, installed ? 1 : 0);
    sqlite3_bind_int(stmt, 2, id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<RepositoryInfo> PackageDb::queryRepositories() {
    std::vector<RepositoryInfo> result;
    const char* sql = "SELECT id, name, url FROM repositories;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RepositoryInfo repo;
        repo.id = sqlite3_column_int(stmt, 0);
        repo.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        repo.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(repo);
    }
    sqlite3_finalize(stmt);
    return result;
}

RepositoryInfo PackageDb::queryRepository(int id) {
    RepositoryInfo repo;
    const char* sql = "SELECT id, name, url FROM repositories WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return repo;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        repo.id = sqlite3_column_int(stmt, 0);
        repo.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        repo.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    }
    sqlite3_finalize(stmt);
    return repo;
}

// --------- package_files table methods ---------

bool PackageDb::insertPackageFile(int package_id, const std::string& filepath) {
    const char* sql = "INSERT INTO package_files (package_id, filepath) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }

    sqlite3_bind_int(stmt, 1, package_id);
    sqlite3_bind_text(stmt, 2, filepath.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool PackageDb::insertPackageFiles(int package_id, const std::vector<std::string>& filepaths) {
    const char* sql_delete = "DELETE FROM package_files WHERE package_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db, sql_delete, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare (delete files) failed: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, package_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Delete files failed: " << sqlite3_errmsg(_db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    for (const auto& fp : filepaths) {
        if (!insertPackageFile(package_id, fp)) return false;
    }
    return true;
}

std::vector<PackageFile> PackageDb::queryPackageFiles(int package_id) {
    std::vector<PackageFile> result;
        
    const char* sql = "SELECT id, package_id, filepath FROM package_files WHERE package_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return result;
    }

    sqlite3_bind_int(stmt, 1, package_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PackageFile pf;
        pf.id = sqlite3_column_int(stmt, 0);
        pf.package_id = sqlite3_column_int(stmt, 1);
        pf.filepath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(pf);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Package> PackageDb::queryPackagesNeedingUpdate() {
    std::vector<Package> result;
    const char* sql =
        "SELECT id, name, version, architecture, maintainer, depends, conflicts, filename, size, md5sum, sha1, sha256, section, description, installed, repository_id, needs_update "
        "FROM packages WHERE needs_update = 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Prepare (queryPackagesNeedingUpdate) failed: " + std::string(sqlite3_errmsg(_db)));
        return result;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Package pkg;
        pkg.id = sqlite3_column_int(stmt, 0);
        pkg.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        pkg.version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        pkg.architecture = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        pkg.maintainer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string dependsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        std::string conflictsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        pkg.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        pkg.size = sqlite3_column_int(stmt, 8);
        pkg.md5sum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        pkg.sha1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        pkg.sha256 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        pkg.section = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        pkg.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        pkg.installed = sqlite3_column_int(stmt, 14) != 0;
        pkg.repository_id = sqlite3_column_int(stmt, 15);

        // Split depends
        pkg.depends.clear();
        size_t start = 0, end;
        while ((end = dependsStr.find(',', start)) != std::string::npos) {
            pkg.depends.push_back(dependsStr.substr(start, end - start));
            start = end + 1;
        }
        if (start < dependsStr.size())
            pkg.depends.push_back(dependsStr.substr(start));

        // Split conflicts
        pkg.conflicts.clear();
        start = 0;
        while ((end = conflictsStr.find(',', start)) != std::string::npos) {
            pkg.conflicts.push_back(conflictsStr.substr(start, end - start));
            start = end + 1;
        }
        if (start < conflictsStr.size())
            pkg.conflicts.push_back(conflictsStr.substr(start));

        result.push_back(pkg);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool PackageDb::deletePackageFiles(int package_id) {
    const char* sql = "DELETE FROM package_files WHERE package_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, package_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool PackageDb::setPackageNeedsUpdate(int package_id, bool needs_update) {
    const char* sql = "UPDATE packages SET needs_update = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, needs_update ? 1 : 0);
    sqlite3_bind_int(stmt, 2, package_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool PackageDb::removePackageFiles(int package_id) {
    const char* sql = "DELETE FROM package_files WHERE package_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }

    sqlite3_bind_int(stmt, 1, package_id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// -- Package update 
bool PackageDb::insertPackageUpdate(const PackageUpdate& update) {
    // 1. Check if an update for this package is already present
    const char* sql_check =
        "SELECT id, new_version FROM package_updates WHERE package_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db, sql_check, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Prepare (check update) failed: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }
    sqlite3_bind_int(stmt, 1, update.package_id);

    int existing_id = 0;
    std::string existing_version;
    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        existing_id = sqlite3_column_int(stmt, 0);
        existing_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        found = true;
    }
    sqlite3_finalize(stmt);

    if (found) {
        if (isNewerVersion(update.new_version, existing_version)) {
            // Update the entry to the newer version
            const char* sql_update =
                "UPDATE package_updates SET old_version = ?, new_version = ?, update_time = ? WHERE id = ?;";
            if (sqlite3_prepare_v2(_db, sql_update, -1, &stmt, nullptr) != SQLITE_OK) {
                _logger.error("Prepare (update update) failed: " + std::string(sqlite3_errmsg(_db)));
                return false;
            }
            sqlite3_bind_text(stmt, 1, update.old_version.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, update.new_version.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, update.update_time.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, existing_id);

            bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
            sqlite3_finalize(stmt);
            return ok;
        } else {
            // Already present and same or newer version, do nothing
            return true;
        }
    } else {
        // Insert if not present
        const char* sql_insert =
            "INSERT INTO package_updates (package_id, package_name, old_version, new_version, update_time) VALUES (?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(_db, sql_insert, -1, &stmt, nullptr) != SQLITE_OK) {
            _logger.error("Prepare (insert update) failed: " + std::string(sqlite3_errmsg(_db)));
            return false;
        }
        sqlite3_bind_int(stmt, 1, update.package_id);
        sqlite3_bind_text(stmt, 2, update.package_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, update.old_version.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, update.new_version.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, update.update_time.c_str(), -1, SQLITE_TRANSIENT);

        bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return ok;
    }
}

// Query updates (optionally filtered by name)
std::vector<PackageUpdate> PackageDb::queryPackageUpdates(int package_id) {
    std::vector<PackageUpdate> result;
    std::string sql = "SELECT id, package_id, package_name, old_version, new_version, update_time FROM package_updates";
    sqlite3_stmt* stmt = nullptr;
    if (package_id > 0) {
        sql += " WHERE package_id = ?";
        if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return result;
        sqlite3_bind_int(stmt, 1, package_id);
    } else {
        if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return result;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PackageUpdate upd;
        upd.id = sqlite3_column_int(stmt, 0);
        upd.package_id = sqlite3_column_int(stmt, 1);
        upd.package_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        upd.old_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        upd.new_version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        upd.update_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        result.push_back(upd);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool PackageDb::deletePackageUpdate(int package_id) {
    const char* sql = "DELETE FROM package_updates WHERE package_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        _logger.error("Prepare (delete) failed: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }
    sqlite3_bind_int(stmt, 1, package_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        _logger.error("Delete failed: " + std::string(sqlite3_errmsg(_db)));
        return false;
    }
    return true;
}

// Helper to get current timestamp as string
std::string PackageDb::getCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}