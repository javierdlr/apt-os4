#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class Logger {
public:
    explicit Logger(bool enabled = true) : enabled_(enabled) {}

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    void debug(const std::string& msg) const {
        if (enabled_) {
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
        if (enabled_) {
            printf(fmt.c_str(), args...);
            printf("\n");
        }
    }

private:
    bool enabled_;
};

#endif // LOGGER_H