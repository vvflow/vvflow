#include "utils.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <unordered_set>

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
        throw std::runtime_error("Error opening pidfile: " + pidfile_path + ": " + errno_str());
    }

    if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        size_t ret = read(fd, buf, sizeof(buf)-1);
        close(fd);
        if (ret <= 0) {
            throw std::runtime_error("Simulation already running: " + sim_caption);
        } else {
            buf[ret] = '\0';
            if (char* nl = strchr(buf, '\n')) {*nl = '\0';}
            throw std::runtime_error("Simulation already running: " + sim_caption + ", pid: " + buf);
        }
    }

    (void)!ftruncate(fd, 0);
}

PidFile::~PidFile() {
    flock(fd, LOCK_UN);
    std::remove(pidfile_path.c_str());
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
        throw std::runtime_error("Socket error: " + errno_str());
    }

    int ret;
    ret = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        throw std::runtime_error("Socket fcntl(O_NONBLOCK) error: " + errno_str());
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    if (sock_path.length() > sizeof(addr.sun_path) - 1) {
        throw std::runtime_error("Socket bind error: " + sock_path + ": filename too long");
    }
    strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);
    unlink(addr.sun_path);
    addr.sun_family = AF_UNIX;

    ret = bind(fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        throw std::runtime_error("Socket bind error: " + sock_path + ": " + errno_str());
    }

    ret = listen(fd, 20);
    if (ret < 0) {
        throw std::runtime_error("Socket listen error: " + sock_path + ": " + errno_str());
    }
}

void ListenSock::accept_all() {
    for (;;) {
        errno = 0;
        int client_fd = accept4(fd, NULL, NULL, O_NONBLOCK);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        } else if (client_fd < 0) {
            throw std::runtime_error("Socket accept error: " + sock_path + ": " + errno_str());
        }
        accepted_fds.insert(client_fd);
    }
}

void ListenSock::broadcast(std::string msg) {
    auto fds(accepted_fds);
    for (int client_fd: fds) {
        errno = 0;
        int ret = write(client_fd, msg.c_str(), msg.length());
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        } else if (ret < msg.length()) {
            close(client_fd);
            accepted_fds.erase(client_fd);
        }
    }
}

ListenSock::~ListenSock() {
    close(fd);
    unlink(sock_path.c_str());
}
