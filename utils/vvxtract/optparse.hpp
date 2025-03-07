#pragma once

namespace opt {
    void parse(int argc, char* const* argv);

    extern int   b_info;
    extern int   b_list;
    extern const char* input;
    extern int argc;
    extern char* const* argv;
}
