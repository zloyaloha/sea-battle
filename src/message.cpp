#include "message.h"

Message::Message(Commands cmd, std::string msg, int pid) {
    _cmd = cmd;
    strcpy(_data, msg.c_str());
    _pid = pid;
}

void send(int fd, Message &msg) {
    write(fd, &msg, sizeof(msg));
}

size_t recv(int fd, Message &msg) {
    return read(fd, &msg, sizeof(msg));
}

