#include "./optparse.hpp"
#include "elementary.h"

#include <string> /* std::string */
#include <cstdlib> /* exit */
#include <getopt.h>

// Mimic http://docopt.org, but without actual parsing
static const char USAGE[] =
R"(
Usage:
  vvflow (-h | --help)
  vvflow (-v | --version)
  vvflow [options] <input> [args]
    <input>          Filename of a Lua script specifying the CFD problem.
    [args]           Arguments passed to the script in the global variable `arg`.
)";

static const char OPTIONS[] =
R"(
Options:
  -h, --help         Show this screen.
  -v, --version      Show version.

  --progress         Print progress during the simulation. (default: false)
  --profile          Save pressure and friction profiles. (default: false)
    Profiles are collected along bodies surfaces and saved
    to the stepdata file.

)";

void opt::parse(int argc, char* const* argv) {
	while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument, 0, 'v'},
            {"help",    no_argument, 0, 'h'},

            {"sensors",  required_argument, 0, 's'},

            {"no-progress", no_argument, &opt::show_progress, 0},
            {   "progress", no_argument, &opt::show_progress, 1},
            {"no-profile",  no_argument, &opt::save_profile, 0},
            {   "profile",  no_argument, &opt::save_profile, 1},

            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "vhs:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v': // -v, --version
            printf("vvplot %s\n", libvvhd_gitinfo);
            printf("revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') exit(0);
            printf("git_diff: %s\n", libvvhd_gitdiff);
            exit(0);
        case 'h': // -h, --help
            printf("Perform the VVD simulation.\n");
            printf("%s", USAGE);
            printf("%s", OPTIONS);
            exit(0);
        case 's':
            opt::sensors_file = optarg;
            break;
        case 0:
            if (long_options[option_index].flag != 0) {
                break;
            }
        case '?':
        default:
            exit(1);
        }
    }

    if (argc-optind < 1) {
        fprintf(stderr, "%s: missing INPUT file\n", argv[0]);
        fprintf(stderr, "%s", USAGE);
        exit(1);
    }

    opt::input = argv[optind+0];

    opt::argc = argc - optind;
    opt::argv = &argv[optind+0];
}
