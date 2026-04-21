#ifndef STRING_H
#define STRING_H

#include <string>

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* str) : std::string(str) {}
    String(const std::string& str) : std::string(str) {}

    void trim() {
        size_t start = find_first_not_of(" \t\n\r");
        if (start == std::string::npos) {
            clear();
            return;
        }
        size_t end = find_last_not_of(" \t\n\r");
        assign(substr(start, end - start + 1));
    }

    int indexOf(char ch) const {
        size_t pos = find(ch);
        return (pos == std::string::npos) ? -1 : pos;
    }

    String substring(int start, int end = -1) const {
        if (end == -1) {
            return String(substr(start));
        } else {
            return String(substr(start, end - start));
        }
    }

    int toInt() const {
        try {
            return std::stoi(*this);
        } catch (...) {
            return 0;
        }
    }
};

#endif
