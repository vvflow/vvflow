#include "./optparse.hpp"
#include "elementary.h"

#include <string> /* std::string */
#include <cstdlib> /* exit */
#include <getopt.h>

// Mimic http://docopt.org, but without actual parsing
static const char USAGE[] =
R"(
Usage:
  vvxtract (-h | --help)
  vvxtract (-v | --version)
  vvxtract [options] <input> [datasets]
    <input>          Filename of an hdf5 (.h5) file, either stepdata or results.
    [datasets]       Datasets to be extracted.
)";

static const char OPTIONS[] =
R"(
Options:
  -h, --help         Show this screen.
  -v, --version      Show version.

  -i, --info         Show general information about the file and exit.
  -l, --list         List all datasets in file and exit.

)";

void opt::parse(int argc, char* const* argv) {
    while (1) {
        static struct option long_options[] =
        {
            {"version", no_argument, 0, 'v'},
            {"help",    no_argument, 0, 'h'},
            {"info",    no_argument, &opt::b_info, 1},
            {"list",    no_argument, &opt::b_list, 1},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "vhil", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'v': // -v, --version
            printf("vvxtract %s\n", libvvhd_gitinfo);
            printf("revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') exit(0);
            printf("git_diff: %s\n", libvvhd_gitdiff);
            exit(0);
        case 'h': // -h, --help
            printf("Extract stepdata and results obtained from vvflow program.\n");
            printf("%s", USAGE);
            printf("%s", OPTIONS);
            exit(0);
        case 'i':
            opt::b_info = 1;
            break;
        case 'l':
            opt::b_list = 1;
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
