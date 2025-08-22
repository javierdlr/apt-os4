#include "repositoryManager.h"

extern APT apt;

bool RepositoryManager::readRepositoryFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening repository file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comment lines starting with '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse lines starting with "deb"
        if (line.substr(0, 3) == "deb") {
            std::istringstream iss(line);
            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(iss, token, ' ')) {
                tokens.push_back(token);
            }

            // Ensure at least three tokens are present
            if (tokens.size() >= 3 && tokens[0] == "deb") {
                Repository rep;
                std::string url = tokens[1];
                std::string distribution = tokens[2];
                std::string repository = tokens[1] + "dists/" + tokens[2];
                rep.baseUrl = repository;
                rep.name = replaceAll(repository, "https://", "");
                rep.name = replaceAll(rep.name, "/", "_");
                repositories.push_back(rep); // Store URL in vector
            }
        }
    }

    file.close();
    return true;
}

bool RepositoryManager::saveRepository(std::string name, std::string content) {
    std::ofstream repository (name, std::ofstream::out);
    if (!repository)
        return false;

    repository << content;
    repository.close();

    return true;
}

std::vector<Repository> RepositoryManager::downloadPackages() {
    // Buffer to store downloaded data
    std::stringstream response;
    std::vector<Repository> result;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing libcurl." << std::endl;
        return result;
    }

    // Set buffer to store response
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Set callback function to write data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

    if (getenv("CURL_VERBOSE") != NULL)
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (apt.ignorePeers())        
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // Skip SSL Verification

    for (const auto& repository : repositories) {
        std::string line;
        Package package;
        Repository rep;
        std::string inRelease = repository.baseUrl +  "/InRelease";
        std::string packagesFile = repository.baseUrl +  "/main/binary-amd64/Packages";
        
        // Download inRelease
        curl_easy_setopt(curl, CURLOPT_URL, inRelease.c_str());
        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Failed to download InRelease file: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return result;
        }
        rep.packages = response.str();
        saveRepository(APT_LISTS + repository.name + "_InRelease", rep.packages);

        response.str("");

        // Download Packages
        curl_easy_setopt(curl, CURLOPT_URL, packagesFile.c_str());

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Failed to download: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return result;
        }

        std::istringstream stream(response.str());
        rep.packages = response.str();
        saveRepository(APT_LISTS + repository.name + "_main_binary-amd64_Packages", rep.packages);

        while (std::getline(stream, line)) {
            // Skip empty lines
            if (line.empty()) continue;

            // Check if the line starts with "Package:"
            if (line.find("Package:") == 0) {
                // If package is not empty, add it to the vector
                if (!package.name.empty()) {
                    packages.push_back(package);
                }
                // Clear package object
                package = Package();
                // Extract package name
                package.name = line.substr(9); // Skip "Package: "
            } else if (!line.empty()) {
                // Parse other fields
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    // Trim leading/trailing whitespaces
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    // Assign values to package object
                    if (key == "Version") {
                        package.version = value;
                    } else if (key == "Architecture") {
                        package.architecture = value;
                    } else if (key == "Maintainer") {
                        package.maintainer = value;
                    } else if (key == "Depends") {
                        package.depends.push_back(value);
                    } else if (key == "Conflicts") {
                        package.conflicts.push_back(value);
                    } else if (key == "Filename") {
                        package.filename = value;
                    } else if (key == "Size") {
                        package.size = std::stoi(value);
                    } else if (key == "MD5sum") {
                        package.md5sum = value;
                    } else if (key == "SHA1") {
                        package.sha1 = value;
                    } else if (key == "SHA256") {
                        package.sha256 = value;
                    } else if (key == "Section") {
                        package.section = value;
                    } else if (key == "Description") {
                        package.description = value;
                    }
                }
            }
        }

        // Add the last package
        if (!package.name.empty()) {
            packages.push_back(package);
        }
        logger.debug("Found " + std::to_string(packages.size()) + " packages in repository " + repository.name);

        result.push_back(rep);
        // Save repository

    }
    return result;
}

bool RepositoryManager::checkPackage(Package newPackage) {
    std::string packageName = APT_PACKAGES + newPackage.name;
    std::string line;
    bool ret = false;
    struct stat st;

    if (stat(packageName.c_str(), &st) == 0) {
        std::ifstream infile (packageName);
        if (!infile)
            return false;

        while (std::getline(infile, line)) {
            std::istringstream iss(line);

            if (!line.empty()) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    // Trim leading/trailing whitespaces
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    // Assign values to package object
                    if (key == "SHA1") {
                        if (newPackage.sha1 != value) {
                            ret = true;
                            break;
                        }
                    }
                }
            }
        }

        infile.close();
     }
     return ret;
}

std::vector<Package> RepositoryManager::searchPackages(std::string searchTerm) {
    DIR *dir;
    struct dirent *ent;
    std::vector<Package> packagesFound;

    if ((dir = opendir(APT_LISTS)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            std::string fileName = APT_LISTS + std::string(ent->d_name);
            if (!endsWith(fileName, "_Packages"))
                continue;
            std::ifstream infile (fileName);
            if (!infile)
                continue;

            Package package;
            std::string line;

            while (std::getline(infile, line)) {
                // Skip empty lines
                if (line.empty()) continue;

                // Check if the line starts with "Package:"
                if (line.find("Package:") == 0) {
                    // Add package if searchTerm is found
                    if (package.name.size() > 0 && package.name.find(searchTerm) != std::string::npos) {
                        package.baseUrl = StringUtils::replace(std::string(ent->d_name), "_Packages", "");
                        package.baseUrl = StringUtils::replaceAll(package.baseUrl, "_", "/");
                        std::vector<std::string> parts = StringUtils::split(package.baseUrl, "/");
                        if (!parts.empty())
                            package.baseUrl = parts[0] + "/" + parts[1];
                        packagesFound.push_back(package);
                    }
                    // Clear package object
                    package = Package();
                    // Extract package name
                    package.name = StringUtils::tolower(line.substr(9)); // Skip "Package: "
                } else if (!line.empty()) {
                    // Parse other fields
                    size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        std::string key = line.substr(0, pos);
                        std::string value = line.substr(pos + 1);
                        // Trim leading/trailing whitespaces
                        key.erase(0, key.find_first_not_of(" \t"));
                        key.erase(key.find_last_not_of(" \t") + 1);
                        value.erase(0, value.find_first_not_of(" \t"));
                        value.erase(value.find_last_not_of(" \t") + 1);
                        // Get only description
                        if (key == "Description") {
                            package.description = value;
                        }
                        else if (key == "Version") {
                            package.version = value;
                        } else if (key == "Filename") {
                            package.filename = value;
                        }
                    }
                }
            }
            // Add last package if searchTerm is found
            if (package.name.size() > 0 && package.name.find(searchTerm) != std::string::npos) {
                packagesFound.push_back(package);
            }
        }
        closedir (dir);
    }
    return packagesFound;

}

std::vector<Package> RepositoryManager::installPackages(std::vector<std::string> packages) {
    std::vector<Package> packagesInstalled;
    for (const auto& package : packages) {
        logger.log("Installing '" + package + "'");
        std::vector<Package> packagesFound = searchPackages(package);
        if (!packagesFound.empty()) {
            // Get first package found - TODO - what if there are multiple files in repositories?
            Package p = packagesFound.front();
            // Create URL from baseUrl
            std::string url = "https://" + p.baseUrl + "/" + p.filename;
            // Create filename to save
            std::string filename = p.filename.substr(p.filename.find_last_of("/\\") + 1);
            std::string outputFile = std::string(APT_PACKAGES) + filename;
            std::string outputDir = std::string(APT_TEMPDIR) + "/"  + p.filename + "/";
            // Download file
            if (verbose())
                logger.debug("Save file to " + outputFile);
            if (downloadFile(url, outputFile)) {
                logger.debug(filename + " downloaded");
                // Install file
                ArExtractor extractor;
                // Extract the .deb file to temporary dir
                logger.debug("Extract " + outputFile + " to " + outputDir + ".. ");
                if (extractor.extract(outputFile.c_str(), outputDir.c_str())) {
                    // File is extracted. Now extract the package
                    logger.debug("Done.");
                    p.installed = true;
                }
                else {
                    logger.error("Failed to extract " + outputFile);
                    // remove the .deb file since it is not installed
                    std::remove(outputFile.c_str());
                }
            }
            else
                logger.error("Error downloading " + url);
            packagesInstalled.push_back(p);
        }
        else {
            logger.error(package + " not found");
        }
    }

    return packagesInstalled;
}

bool RepositoryManager::downloadFile(const std::string& url, const std::string& filename) {
    curl_global_init(CURL_GLOBAL_ALL);
    // Initialize libcurl easy handle
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl." << std::endl;
        return false;
    }

    if (getenv("CURL_VERBOSE") != NULL)
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (apt.ignorePeers())
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // Skip SSL Verification

    // Open the output file
    FILE *file = fopen(filename.c_str(), "wb");
    if (!file) {
        std::cerr << "Failed to open file for writing." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    // Set URL to download
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set the file handle as the target for the transfer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    // disable progress meter, set to 0L to enable it
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to download file: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        fclose(file);
        return false;
    }

    // Close the file and cleanup
    curl_easy_cleanup(curl);
    fclose(file);

    return true;
}