#include "./exear.hpp"
#include <sys/stat.h>

const char* Exear::exear_header =
"#!/bin/bash\n"
"T=`mktemp -d`\n"
"tar -xf $0 --exclude=#* -C $T"
" && cd $T"
" && (. main.sh)\n"
"rm -rf $T\n"
"exit 0\n";

Exear::Exear(std::string filename):
    filename(filename)
{
    a = archive_write_new();
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, filename.c_str());

    e = archive_entry_new();
    archive_entry_set_pathname(e, exear_header);
    archive_entry_set_size(e, 0);
    archive_entry_set_filetype(e, AE_IFREG);
    archive_entry_set_perm(e, 0644);
    archive_write_header(a, e);
    archive_entry_clear(e);
}

Exear::~Exear()
{
    archive_entry_free(e);
    archive_write_close(a);
    archive_write_free(a);
    chmod(filename.c_str(), 0755);
}

// void Exear::append(const char* filename, std::vector<char> data, int perm)
// {
//     archive_entry_set_pathname(e, filename);
//     archive_entry_set_size(e, data.size());
//     archive_entry_set_filetype(e, AE_IFREG);
//     archive_entry_set_perm(e, perm);
//     archive_write_header(a, e);
//     archive_write_data(a, data.data(), data.size());
//     archive_entry_clear(e);
// }

void Exear::append(const char* filename, std::string str, int perm)
{
    archive_entry_set_pathname(e, filename);
    archive_entry_set_size(e, str.length());
    archive_entry_set_filetype(e, AE_IFREG);
    archive_entry_set_perm(e, perm);
    archive_write_header(a, e);
    archive_write_data(a, str.c_str(), str.length());
    archive_entry_clear(e);
}
