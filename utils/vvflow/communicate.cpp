#include "communicate.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>

std::string errno_str() {
    return std::string(std::strerror(errno));
}

PidFile::PidFile(const std::string& sim_caption):
    fd(0),
    sim_caption(sim_caption),
    pidfile_path("." + sim_caption + ".pid")
{
    fd = open(pidfile_path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        throw std::runtime_error("error opening pidfile: " + pidfile_path + ": " + errno_str());
    }

    if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        size_t ret = read(fd, buf, sizeof(buf)-1);
        close(fd);
        if (ret <= 0) {
            throw std::runtime_error("simulation already running: " + sim_caption);
        } else {
            buf[ret] = '\0';
            throw std::runtime_error("simulation already running: " + sim_caption + ", pid: " + buf);
        }
    }

    (void)!ftruncate(fd, 0);
}

void PidFile::write() {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d\n", getpid());
    (void)!::write(fd, buf, strlen(buf));
}

ListenSock::ListenSock(const std::string& sim_caption):
    fd(0),
    sim_caption(sim_caption),
    sock_path("." + sim_caption + ".sock")
{
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        throw std::runtime_error("socket error: " + errno_str());
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    if (sock_path.length() > sizeof(addr.sun_path) - 1) {
        throw std::runtime_error("socket error: " + sock_path + ": filename too long");
    }
    strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);
    unlink(addr.sun_path);
    addr.sun_family = AF_UNIX;

    int ret = bind(fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        if (errno == EADDRINUSE) {
            throw std::runtime_error("simulation already running: " + sim_caption);
        } else {
            throw std::runtime_error("socket bind error: " + sock_path + ": " + errno_str());
        }
    }

    ret = listen(fd, 20);
    if (ret < 0) {
        throw std::runtime_error("socket listen error: " + sock_path + ": " + errno_str());
    }
}

ListenSock::~ListenSock() {
    unlink(sock_path.c_str());
}
