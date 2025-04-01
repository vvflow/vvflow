#pragma once

#include <string>
#include <unordered_set>
#include <vector>

class PidFile {
public:
    PidFile(const std::string& sim_caption);
    ~PidFile();
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
    void accept_all();
    void broadcast(std::string msg);
private:
    int fd;
    std::string sim_caption;
    std::string sock_path;
    std::unordered_set<int> accepted_fds;
};

// class ClientSock {
//     public:
//         ClientSock(const char* filename);
// };
