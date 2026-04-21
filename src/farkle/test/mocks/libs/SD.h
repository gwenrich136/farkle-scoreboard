#ifndef MOCK_SD_H
#define MOCK_SD_H

#include <Arduino.h>
#include "String.h"
#include <string>
#include <vector>
#include <map>

#define FILE_READ 0
#define FILE_WRITE 1

class File {
public:
    File() : _isOpen(false), _position(0), _fileName("") {}
    File(std::string name, std::string content) : _isOpen(true), _position(0), _content(content), _fileName(name) {}

    operator bool() const { return _isOpen; }

    int available() {
        if (!_isOpen) return 0;
        return _content.length() - _position;
    }

    String readStringUntil(char terminator) {
        if (!_isOpen || _position >= _content.length()) return String("");

        size_t endPos = _content.find(terminator, _position);
        if (endPos == std::string::npos) {
            std::string res = _content.substr(_position);
            _position = _content.length();
            return String(res.c_str());
        } else {
            std::string res = _content.substr(_position, endPos - _position);
            _position = endPos + 1;
            return String(res.c_str());
        }
    }

    void print(const char* str) {
        if (_isOpen) _writeBuffer += str;
    }

    void print(int val) {
        if (_isOpen) _writeBuffer += std::to_string(val);
    }

    void println(const char* str) {
        if (_isOpen) { _writeBuffer += str; _writeBuffer += "\n"; }
    }

    void println(int val) {
        if (_isOpen) { _writeBuffer += std::to_string(val); _writeBuffer += "\n"; }
    }

    void close();

    std::string _writeBuffer;
    bool _isOpen;
    size_t _position;
    std::string _content;
    std::string _fileName;
};

class SDClass {
public:
    bool begin(int csPin) { return true; }

    bool exists(const char* filepath) {
        return _files.find(filepath) != _files.end();
    }

    File open(const char* filepath, int mode) {
        if (mode == FILE_READ) {
            if (exists(filepath)) {
                return File(filepath, _files[filepath]);
            }
            return File();
        } else {
            // FILE_WRITE overrides for now in this mock
            return File(filepath, "");
        }
    }

    void remove(const char* filepath) {
        _files.erase(filepath);
    }

    // Test helpers
    void mockSetFileContent(const char* filepath, const char* content) {
        _files[filepath] = content;
    }

    std::string mockGetFileContent(const char* filepath) {
        return _files[filepath];
    }

    void mockSaveFileContent(const char* filepath, const std::string& content) {
        _files[filepath] = content;
    }

private:
    std::map<std::string, std::string> _files;
};

extern SDClass SD;

#endif
