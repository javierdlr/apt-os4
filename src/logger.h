#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class Logger {
public:
    explicit Logger(bool verbose = true) : _verbose(verbose) {}

    void setVerbose(bool enabled) { _verbose = enabled; }
    bool isVerbose() const { return _verbose; }

    void debug(const std::string& msg) const {
        if (_verbose) {
            std::cout << msg << std::endl;
        }
    }

    void log(const std::string& msg) const {
        std::cout << msg << std::endl;
    }

    void error(const std::string& msg) const {
        std::cerr << "Error: " << msg << std::endl;
    }

    // Optional: log with formatting (like printf)
    template<typename... Args>
    void logf(const std::string& fmt, Args... args) const {
        if (_verbose) {
            printf(fmt.c_str(), args...);
            printf("\n");
        }
    }

private:
    bool _verbose;
};

#endif // LOGGER_H