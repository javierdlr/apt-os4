#ifndef __STINGUTILS_H__
#define __STINGUTILS_H__

#include <cctype>
#include <string>
#include <sstream>
#include <vector>

class StringUtils {
public:
    static std::string tolower(std::string str) {
        for (auto &c: str) {
            c = std::tolower(c);
        }
        return str;
    }

    static std::string replace(const std::string &str, const std::string &find, const std::string &replace) {
        std::string retval = str;
        size_t start_pos = retval.find(find);
        if (start_pos == std::string::npos)
            return str;
        retval.replace(start_pos, find.length(), replace);
        return retval;
    }

    static std::string replaceAll(const std::string &str, const std::string &from, const std::string &to) {
        std::string retval = str;
        if (from.empty())
            return str;
        size_t start_pos = 0;
        while ((start_pos = retval.find(from, start_pos)) != std::string::npos) {
            retval.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
        return retval;
    }

    static bool endsWith(std::string const &value, std::string const &ending) {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    static std::vector <std::string> split(std::string str, std::string token) {
        std::vector<std::string> result;
        while (str.size()) {
            unsigned int index = str.find(token);
            if (index != std::string::npos) {
                result.push_back(str.substr(0, index));
                str = str.substr(index + token.size());
                if (str.size() == 0)result.push_back(str);
            } else {
                result.push_back(str);
                str = "";
            }
        }
        return result;
    }
};

#endif