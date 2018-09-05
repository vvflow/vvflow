#pragma once

// /* libarchive */
// #include <archive.h>
// #include <archive_entry.h>

/* std::... */
#include <string>
#include <vector>
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
    void plot(const std::string& filename);

    void add(const std::string& filename, const std::string& str) {
        files[filename] = str;
    }

    friend std::ostream& operator<< (Gnuplotter& gp, const std::string& str) {
        return gp.script << str;
    }

private:
    std::stringstream script;
    std::map < std::string, std::string > files;
};
