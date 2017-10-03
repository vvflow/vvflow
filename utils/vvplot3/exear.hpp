#pragma once

/* libarchive */
#include <archive.h>
#include <archive_entry.h>

/* std::... */
#include <string>
#include <vector>

class Exear {
public:
    Exear(std::string filename);
    ~Exear();
    Exear(const Exear&) = delete;
    Exear(Exear&&) = delete;
    Exear& operator= (const Exear&) = delete;
    Exear& operator= (Exear&&) = delete;

    // void append(const char* filename, std::vector<char> data, int perm = 0644);
    void append(const char* filename, const void* data, size_t len, int perm = 0644);
    void append(const char* filename, const std::string& str, int perm = 0644) {
        append(filename, str.c_str(), str.size(), perm);
    }

private:
    std::string filename;
    struct archive *a;
    struct archive_entry *e;

    static const char* exear_header;
};
