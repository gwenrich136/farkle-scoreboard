#ifndef SD_MOCK_H
#define SD_MOCK_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>
#include <cstring>
#include <Arduino.h>

#define FILE_READ 0
#define FILE_WRITE 1
#define O_READ 0
#define O_WRITE 1
#define O_CREAT 2

class File {
private:
    std::string _path;
    std::fstream* _stream;
    bool _isDirectory;
    uint32_t _size;
    bool _is_open;

public:
    File() : _path(""), _stream(nullptr), _isDirectory(false), _size(0), _is_open(false) {}

    // Rule of three to avoid double free
    File(const File& other) = delete;
    File& operator=(const File& other) = delete;

    File(File&& other) noexcept : _path(other._path), _stream(other._stream), _isDirectory(other._isDirectory), _size(other._size), _is_open(other._is_open) {
        other._stream = nullptr;
        other._is_open = false;
    }

    File& operator=(File&& other) noexcept {
        if (this != &other) {
            close();
            _path = other._path;
            _stream = other._stream;
            _isDirectory = other._isDirectory;
            _size = other._size;
            _is_open = other._is_open;
            other._stream = nullptr;
            other._is_open = false;
        }
        return *this;
    }

    File(std::string path, uint8_t mode) : _path(path), _isDirectory(false), _is_open(false) {
        _stream = new std::fstream();
        std::ios_base::openmode openmode;

        // Ensure parent directories exist
        size_t last_slash = path.find_last_of('/');
        if (last_slash != std::string::npos) {
            std::string dir = path.substr(0, last_slash);
            std::string command = "mkdir -p " + dir;
            system(command.c_str());
        }

        if (mode == FILE_READ) {
            openmode = std::ios::in | std::ios::binary;
        } else {
            // For SD library, FILE_WRITE typically appends or creates.
            openmode = std::ios::out | std::ios::in | std::ios::app | std::ios::binary;
        }

        _stream->open(path, openmode);
        if (!_stream->is_open() && mode == FILE_WRITE) {
            // Try creating it if it didn't exist
            _stream->open(path, std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
        }

        if (_stream->is_open()) {
            _is_open = true;
            _stream->seekg(0, std::ios::end);
            _size = _stream->tellg();
            _stream->seekg(0, std::ios::beg);
        } else {
            delete _stream;
            _stream = nullptr;
        }
    }

    ~File() {
        close();
    }

    operator bool() { return _is_open; }

    void close() {
        if (_stream) {
            _stream->close();
            delete _stream;
            _stream = nullptr;
        }
        _is_open = false;
    }

    int available() {
        if (!_is_open || !_stream) return 0;
        long current = _stream->tellg();
        _stream->seekg(0, std::ios::end);
        long end = _stream->tellg();
        _stream->seekg(current, std::ios::beg);
        return end - current;
    }

    int read() {
        if (!_is_open || !_stream) return -1;
        return _stream->get();
    }

    size_t read(void *buf, size_t nbyte) {
        if (!_is_open || !_stream) return 0;
        _stream->read((char*)buf, nbyte);
        return _stream->gcount();
    }

    std::string readStringUntil(char terminator) {
        if (!_is_open || !_stream) return "";
        std::string res;
        std::getline(*_stream, res, terminator);
        return res;
    }

    size_t write(uint8_t b) {
        if (!_is_open || !_stream) return 0;
        _stream->put(b);
        return 1;
    }

    size_t write(const uint8_t *buf, size_t size) {
        if (!_is_open || !_stream) return 0;
        _stream->write((const char*)buf, size);
        return size;
    }

    size_t print(const char* str) {
        if (!_is_open || !_stream) return 0;
        _stream->write(str, strlen(str));
        return strlen(str);
    }

    size_t print(long n) {
        std::string s = std::to_string(n);
        return print(s.c_str());
    }

    size_t println(const char* str) {
        size_t n = print(str);
        n += print("\n");
        return n;
    }

    bool seek(uint32_t pos) {
        if (!_is_open || !_stream) return false;
        _stream->seekg(pos, std::ios::beg);
        _stream->seekp(pos, std::ios::beg);
        return true;
    }

    uint32_t position() {
        if (!_is_open || !_stream) return 0;
        return _stream->tellg();
    }

    uint32_t size() {
        return _size;
    }

    void flush() {
        if (_stream) {
            _stream->flush();
        }
    }
};

class SDClass {
private:
    std::string _base_path;

public:
    SDClass() : _base_path("mock_sd_root/") {
        system(("mkdir -p " + _base_path).c_str());
    }

    bool begin(uint8_t csPin) {
        return true;
    }

    File open(const char *filename, uint8_t mode = FILE_READ) {
        std::string full_path = _base_path + (filename[0] == '/' ? filename + 1 : filename);
        return File(full_path, mode);
    }

    File open(const std::string &filename, uint8_t mode = FILE_READ) {
        return open(filename.c_str(), mode);
    }

    bool exists(const char *filepath) {
        std::string full_path = _base_path + (filepath[0] == '/' ? filepath + 1 : filepath);
        struct stat buffer;
        return (stat(full_path.c_str(), &buffer) == 0);
    }

    bool exists(const std::string &filepath) {
        return exists(filepath.c_str());
    }

    bool remove(const char *filepath) {
        std::string full_path = _base_path + (filepath[0] == '/' ? filepath + 1 : filepath);
        return ::remove(full_path.c_str()) == 0;
    }

    bool remove(const std::string &filepath) {
        return remove(filepath.c_str());
    }

    bool mkdir(const char *filepath) {
        std::string full_path = _base_path + (filepath[0] == '/' ? filepath + 1 : filepath);
        return system(("mkdir -p " + full_path).c_str()) == 0;
    }

    bool rmdir(const char *filepath) {
        std::string full_path = _base_path + (filepath[0] == '/' ? filepath + 1 : filepath);
        return system(("rm -rf " + full_path).c_str()) == 0;
    }

    // Custom method for tests to clear the SD card
    void _clearMockSD() {
        system(("rm -rf " + _base_path + "*").c_str());
    }
};

extern SDClass SD;

#endif
