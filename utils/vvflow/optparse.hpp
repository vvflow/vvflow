#pragma once

namespace opt {
    void parse(int argc, char **argv);

    extern int   save_profile;
    extern int   show_progress;
    extern const char* input;
    extern const char* sensors_file;
}
