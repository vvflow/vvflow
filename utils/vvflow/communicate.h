#pragma once

#include <string>

class PidFile {
public:
    PidFile(const std::string& sim_caption);
    void write();
private:
    int fd;
    std::string sim_caption;
    std::string pidfile_path;
};

class ListenSock {
public:
    ListenSock(const std::string& sim_caption);
    ~ListenSock();
private:
    std::string sim_caption;
    std::string sock_path;
    int fd;
};

// class ClientSock {
//     public:
//         ClientSock(const char* filename);
// };
