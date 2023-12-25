#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>
#include "battlefield.h"

std::string myfifo_write_default = "/tmp/myfifo_c-s_def";

std::string myfifo_read = "/tmp/myfifo_s-c_" + std::to_string(getpid());
std::string myfifo_write = "/tmp/myfifo_c-s_" + std::to_string(getpid());

int fdR, fdW;
std::string username = "";

bool default_connection() {
    mkfifo(myfifo_read.c_str(), 0666);
    mkfifo(myfifo_write.c_str(), 0666);
    mkfifo(myfifo_write_default.c_str(), 0666);
    std::cout << "Trying connect to server" << std::endl;
    int fd_write_default = open(myfifo_write_default.c_str(), O_WRONLY);
    std::cout << "Open default write fifo" << std::endl;
    std::string reply;
    Message msg_to_server(Commands::connect, "", getpid());
    Message reply_from_server(Commands::fail, reply, 0);
    send(fd_write_default, msg_to_server);

    if (close(fd_write_default) == -1) {
        throw std::logic_error("bad with close");
    }

    fdR = open(myfifo_read.c_str(), O_RDONLY);
    fdW = open(myfifo_write.c_str(), O_WRONLY);
    std::cout << "Open write read fifo" << std::endl;
    recv(fdR, reply_from_server);

    if (reply_from_server._cmd == success) {
        std::cout << "Succesfully connected" << std::endl;
        return true;
    } else {
        close(fdR);
        close(fdW);
        std::cout << "Problem with connection" << std::endl;
        return false;
    }
}

void start_game(Battlefield &btf) {
    while (btf.one_amount() != 4 || btf.two_amount() || 3 && btf.three_amount() != 2 || btf.four_amount() || 1) {
        btf.print();
        std::cout << "Please, enter direction (0 for horizontal, 1 for " \
            "vertical), coordinates of ship and his type.\nExample: 0 A 1 4" << std::endl;
        char column; int row; int type; int direction;
        std::cin >> direction >> column >> row >> type;
        std::string msg = std::to_string(direction) + ' ' + column + ' ' + std::to_string(row) + ' ' + std::to_string(type);
        if (btf.place_ship(column, row, Direction(direction), ShipType(type))) {
            Message msg_to_server(Commands::place_ship, msg, getpid());
            send(fdW, msg_to_server);
        } else {
            std::cout << "You have done something wrong" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // Battlefield btf;
    // start_game(btf);
    default_connection();
    std::string input;
    while (1) {
        std::cout << "Write command" << std::endl;
        std::cin >> input;
        if (input == "login") {
            if (username != "") {
                std::cout << "You are already autorized in " + username << std::endl;
                continue;
            }
            std::string login, reply;
            std::cin >> login;
            Message msg_to_server(Commands::login, login, getpid());
            Message reply_from_server(Commands::login, reply, 0);
            send(fdW, msg_to_server);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "Accout succesfuly autorized " << reply_from_server._data << std::endl;
                username = login;
            } else if (reply_from_server._cmd == fail) {
                std::cout << reply_from_server._data << std::endl;
            }
        } else if (input == "make") {
            std::string login, reply;
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
        } else if (input == "find") {
            std::string login;
            if (username == "") {
                std::cout << "You are not authorized" << std::endl;
                continue;
            }
            std::cin >> login;
            Message msg_to_server(Commands::find, login, getpid());
            send(fdW, msg_to_server);
            Message reply_from_server(create_user, "", -1);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << reply_from_server._data << std::endl;
                Battlefield own_battlefield;
                Battlefield opponent_battlefield;
                start_game(own_battlefield);
            } else {
                std::cout << reply_from_server._data << std::endl;
            }
        }
        std::cout << std::endl;
    }
}