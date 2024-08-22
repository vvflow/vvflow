#pragma once

namespace opt {
    void parse(int argc, char* const* argv);

    extern int   save_profile;
    extern int   show_progress;
    extern const char* input;
    extern const char* sensors_file;
    extern int argc;
    extern char* const* argv;
}
