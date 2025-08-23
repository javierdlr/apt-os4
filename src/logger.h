#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class Logger {
public:
    explicit Logger(bool verbose = true) : verbose_(verbose) {}

    void setVerbose(bool enabled) { verbose_ = enabled; }
    bool isVerbose() const { return verbose_; }

    void debug(const std::string& msg) const {
        if (verbose_) {
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
        if (verbose_) {
            printf(fmt.c_str(), args...);
            printf("\n");
        }
    }

private:
    bool verbose_;
};

#endif // LOGGER_H