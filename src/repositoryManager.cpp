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
                std::cout << "Repository: URL: " << rep.baseUrl << ", Name: " << rep.name  << std::endl;
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
    // Parse the response
    std::string line;
    Package package;
    // Buffer to store downloaded data
    std::stringstream response;
    std::vector<Repository> result;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing libcurl." << std::endl;
        return result;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Set callback function to write data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

    // Set buffer to store response
    if (getenv("CURL_VERBOSE") != NULL)
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (apt.ignorePeers())        
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // Skip SSL Verification

    for (const auto& repository : repositories) {
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
        std::cout << "Found " << packages.size() << " packages in repository " << repository.name << std::endl;

        result.push_back(rep);
        // Save repository

    }
    return result;
}