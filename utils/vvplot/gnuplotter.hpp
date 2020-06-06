#pragma once

// /* libarchive */
// #include <archive.h>
// #include <archive_entry.h>

/* std::... */
#include <string>
#include <vector>
#include <memory> //unique_ptr
#include <iostream>
#include <sstream>
#include <map>

class Gnuplotter {
public:
    Gnuplotter();
    ~Gnuplotter() = default;
    Gnuplotter(const Gnuplotter&) = delete;
    Gnuplotter(Gnuplotter&&) = delete;
    Gnuplotter& operator= (const Gnuplotter&) = delete;
    Gnuplotter& operator= (Gnuplotter&&) = delete;

    void load(const std::string& filename);
    void save(const std::string& filename);
    void exec(const std::string& filename);

    void add(const std::string& filename, const std::string& str) {
        files[filename] = str;
    }
    const std::string* get(const std::string& filename) {
        const auto it = files.find(filename);
        if (it == files.end())
            return nullptr;
        else
            return &it->second;
    }

    friend std::ostream& operator<< (Gnuplotter& gp, const std::string& str) {
        return gp.script << str;
    }

private:
    std::stringstream script;
    std::map < std::string, std::string > files;
};

template<typename ... Args>
std::string strfmt(const char* format, Args ... args)
{
    size_t size = snprintf( nullptr, 0, format, args ... ) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format, args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}
