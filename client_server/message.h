#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>

enum Commands {
    login = 0,
    create_user = 1,
    success = 3,
    fail = 4,
    stats = 5,
    connect = 6
};

struct Message {
    Message(Commands cmd, std::string msg, int pid) {
        _cmd = cmd;
        strcpy(_data, msg.c_str());
        _pid = pid;
    }
    Message() = default;
    Commands _cmd;
    char _data[64];
    int _pid;
};

void send(int fd, Message &msg) {
    write(fd, &msg, sizeof(msg));
}

size_t recv(int fd, Message &msg) {
    return read(fd, &msg, sizeof(msg));
}