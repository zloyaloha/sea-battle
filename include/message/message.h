#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>

enum Commands {
    login,
    create_user,
    success,
    fail,
    stats,
    connect,
    find,
    place_ship,
    ready_to_play,
    kill_ship,
    end_game,
    clear,
    disconnect
};


struct Message {
    Message(Commands cmd, std::string msg, int pid);
    Message() : _cmd(fail), _data(0), _pid(0) {}
    Commands _cmd;
    char _data[64];
    int _pid;
};

void send(int fd, Message &msg);
size_t recv(int fd, Message &msg);
