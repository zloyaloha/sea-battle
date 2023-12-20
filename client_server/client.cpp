#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>

std::string myfifo_read = "/tmp/myfifo_read";
std::string myfifo_write = "/tmp/myfifo_write";

int main(int argc, char* argv[]) {
    std::string username = "";
    int fd_read;
    int fd_write;
    mkfifo(myfifo_read.c_str(), 0666);
    mkfifo(myfifo_write.c_str(), 0666);
    std::string input;
    int fdR = open(myfifo_read.c_str(), O_RDONLY);
    int fdW = open(myfifo_write.c_str(), O_WRONLY);
    while (1) {
        std::cout << "Write command" << std::endl;
        std::cin >> input;
        if (input == "login") {
            if (username != "") {
                std::cout << "You are already autorized in " + username << std::endl;
                continue;
            }
            std::string login, reply;
            std::cout << "Write username\n";
            std::cin >> login;
            Message msg_to_server(Commands::login, login, getpid());
            Message reply_from_server(Commands::login, reply, 0);
            send(fdW, msg_to_server);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "Accout succesfuly autorized " << reply_from_server._data << std::endl;
                username = login;
            } else if (reply_from_server._cmd == fail) {
                std::cout << "Account isn't exist " << reply_from_server._data << std::endl;
            }
        } else if (input == "make") {
            std::string login, reply;
            std::cout << "Write username\n";
            std::cin >> login;
            Message msg_to_server(Commands::create_user, login, getpid());
            Message reply_from_server(create_user, reply, -1);
            send(fdW, msg_to_server);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "Accout succesfuly created " << reply_from_server._data << std::endl;
            } else if (reply_from_server._cmd == fail) {
                std::cout << "Account is exist already " << reply_from_server._data << std::endl;
            }
        } else if (input == "stats") {
            if (username == "") {
                std::cout << "You are not authorized" << std::endl;
                continue;
            }
            Message msg_to_server(Commands::stats, username, getpid());
            send(fdW, msg_to_server);
            Message reply_from_server(create_user, "", -1);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "\nStats of: " << username << std::endl;
                std::cout << "Win-Lose: " << reply_from_server._data << std::endl;
            } else if (reply_from_server._cmd == fail) {
                std::cout << "something wrong " << reply_from_server._data << std::endl;
            }
        }
        std::cout << std::endl;
    }
    close(fdR);
    close(fdW);
}