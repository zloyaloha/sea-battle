#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>

std::string myfifo = "/tmp/myfifo";

int main(int argc, char* argv[]) {
    int fd;
    mkfifo(myfifo.c_str(), 0666);
    std::string input;
    std::cout << "Write command" << std::endl;
    while (1) {
        std::cin >> input;
        if (input == "login") {
            std::string login;
            std::cout << "Write username\n";
            std::cin >> login;
            Message msg_to_server(Commands::login, login);
            fd = open(myfifo.c_str(), O_WRONLY);
            send(fd, msg_to_server);
            close(fd);
        } else if (input == "make") {
            std::string login;
            std::cout << "Write username\n";
            std::cin >> login;
            Message msg_to_server(Commands::create_user, login);
            fd = open(myfifo.c_str(), O_WRONLY);
            send(fd, msg_to_server);
            close(fd);
        }
    }
}