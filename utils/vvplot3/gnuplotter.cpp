#include "./gnuplotter.hpp"
#include <sys/stat.h>

/* libarchive */
#include <archive.h>
#include <archive_entry.h>

Gnuplotter::Gnuplotter():
    script(),
    files()
{}

static void archive(
    struct archive *a,
    const std::string& name,
    const std::string& data,
    int perm = 0644)
{
    struct archive_entry *e = archive_entry_new();
    archive_entry_set_pathname(e, name.c_str());
    archive_entry_set_size(e, data.size());
    archive_entry_set_filetype(e, AE_IFREG);
    archive_entry_set_perm(e, perm);
    archive_write_header(a, e);
    archive_write_data(a, data.c_str(), data.size());
    archive_entry_free(e);
}

void Gnuplotter::save(const std::string& filename)
{
    struct archive *a = archive_write_new();

    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, filename.c_str());

    archive(a, "plot.gp", script.str(), 0755);
    for (auto& f: files) {
        archive(a, f.first, f.second);
    }

    archive_write_close(a);
    archive_write_free(a);
}

void Gnuplotter::load(const std::string& filename)
{
    ssize_t size;

    struct archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    int rc = archive_read_open_filename(a, filename.c_str(), 16384);
    if (rc != ARCHIVE_OK) {
        std::string err = "Error opening '" + filename + "': ";
        err += archive_error_string(a);
        throw std::runtime_error(err);
    }

    while (true) {
        int rc;
        struct archive_entry *entry;
        rc = archive_read_next_header(a, &entry);
        if (rc == ARCHIVE_FATAL) {
            std::string err = "Error reading '" + filename + "': ";
            err += archive_error_string(a);
            throw std::runtime_error(err);
        } else if (rc == ARCHIVE_EOF) {
            break;
        }

        std::string pathname = archive_entry_pathname(entry);
        size_t esize = archive_entry_size(entry);
        char buf[esize];

        size_t rsize = archive_read_data(a, buf, esize);
        if (rsize < 0) {
            std::string err = "Error reading '" + filename + "' (" + pathname + "): ";
            err += archive_error_string(a);
            throw std::runtime_error(err);
        }

        if (pathname == "plot.gp") {
            script.write(buf, rsize);
        } else {
            files[pathname] = std::string(buf, rsize);
        }
    }

    archive_read_free(a);
}

void Gnuplotter::plot(const std::string& filename)
{
    // TODO
}
