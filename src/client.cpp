#include "client.h"

std::string myfifo_write_default = "/tmp/myfifo_c-s_def";

Client::Client(int pid) {
    std::string username = "";
    std::string myfifo_read = "/tmp/myfifo_s-c_" + std::to_string(pid);
    std::string myfifo_write = "/tmp/myfifo_c-s_" + std::to_string(pid);
    mkfifo(myfifo_read.c_str(), 0666);
    mkfifo(myfifo_write.c_str(), 0666);
    mkfifo(myfifo_write_default.c_str(), 0666);
    int fd_write_default = open(myfifo_write_default.c_str(), O_WRONLY);
    std::string reply;
    Message msg_to_server(Commands::connect, "", getpid());
    Message reply_from_server(Commands::fail, reply, 0);
    send(fd_write_default, msg_to_server);
    if (close(fd_write_default) == -1) {
        throw std::logic_error("bad with close");
    }
    fdR = open(myfifo_read.c_str(), O_RDONLY);
    fdW = open(myfifo_write.c_str(), O_WRONLY);
    recv(fdR, reply_from_server);
    if (reply_from_server._cmd == success) {
        return;
    } else {
        close(fdR);
        close(fdW);
        std::cout << "Problem with connection" << std::endl;
        exit(1);
    }
}

bool Client::startGame(Battlefield &btf) {
    while (btf.one_amount() != 4 && btf.two_amount() != 3 && btf.three_amount() != 2 && btf.four_amount() != 1) {
        btf.print();
        std::cout << "Please, enter direction (0 for horizontal, 1 for " \
            "vertical), coordinates of ship and his type.\nExample: 0 A 1 4" << std::endl;
        char column; int row; int type; int direction;
        std::cin >> direction >> column >> row >> type;
        std::string msg = std::to_string(direction) + ' ' + column + ' ' + std::to_string(row) + ' ' + std::to_string(type);
        if (btf.place_ship(column, row, Direction(direction), ShipType(type)) == 0) {
            Message msg_to_server(Commands::place_ship, msg, getpid());
            send(fdW, msg_to_server);
            Message reply(Commands::fail, "", -1);
            recv(fdR, reply);
            if (reply._cmd != success) {
                if (reply._cmd == disconnect) {
                    return 0;
                }
                throw std::logic_error("Not placed");
            } 
        } else {
            std::cout << "\nYou have done something wrong" << std::endl;
        }
    }
    btf.print();
    Message msg_to_server(Commands::ready_to_play, "", getpid());
    send(fdW, msg_to_server);
    return 1;
}

bool Client::gameInProcces(Battlefield &own, Battlefield &opponent, int number) {
    if (number == 1) {
        while (1) {
            own.print(); opponent.print();
            std::string column; int row;
            std::cout << "Please, enter coordinates of ship.\nExample: B 7 " << std::endl;
            std::cin >> column >> row;
            std::string msg = column + ' ' + std::to_string(row);
            Message msg_to_server(Commands::kill_ship, msg, getpid());
            Message is_killed(Commands::fail, "", -1);
            send(fdW, msg_to_server);
            recv(fdR, is_killed);
            if (is_killed._cmd == success) {
                std::cout << is_killed._data << std::endl;
                opponent.set(is_killed._data[0], is_killed._pid, '+');
            } else if (is_killed._cmd == end_game) {
                std::cout << is_killed._data << std::endl;
                return 1;
            } else if (is_killed._cmd == fail) {
                opponent.set(column[0], row, '*');
                std::cout << is_killed._data << std::endl;
            } else if (is_killed._cmd == disconnect ){
                return 0;
            }
            own.print(); opponent.print();
            std::cout << "Waiting opponent" << std::endl;
            Message opponent_move(Commands::fail, "", -1);
            recv(fdR, opponent_move);
            std::cout << opponent_move._cmd << ' ' << opponent_move._data << ' ' << opponent_move._pid << std::endl;
            if (opponent_move._cmd == success) {
                std::cout << opponent_move._data << std::endl;
                own.set(opponent_move._data[0], opponent_move._pid, '+');
            } else if (opponent_move._cmd == end_game) {
                std::cout << opponent_move._data << std::endl;
                return 1;
            } else if (opponent_move._cmd == fail) {
                own.set(opponent_move._data[0], opponent_move._pid, '*');
                std::cout << opponent_move._data << std::endl;
            } else if (opponent_move._cmd == disconnect) {
                return 0;
            }
        }
    } else {
        while (1) {
            own.print(); opponent.print();
            std::cout << "Waiting opponent" << std::endl;
            Message opponent_move(Commands::fail, "", -1);
            recv(fdR, opponent_move);
            if (opponent_move._cmd == success) {
                own.set(opponent_move._data[0], opponent_move._pid, '+');
                std::cout << opponent_move._data << std::endl;
            } else if (opponent_move._cmd == end_game) {
                std::cout << opponent_move._data << std::endl;
                return 1;
            } else if (opponent_move._cmd == fail) {
                own.set(opponent_move._data[0], opponent_move._pid, '*');
                std::cout << opponent_move._data << std::endl;
            } else if (opponent_move._cmd == disconnect) {
                return 0;
            } else {
                throw std::logic_error("unknown command");
            }
            own.print(); opponent.print();
            std::string column; int row;
            std::cout << "Please, enter coordinates of ship.\nExample: B 7 " << std::endl;
            std::cin >> column >> row;
            std::string msg = column + ' ' + std::to_string(row);
            Message msg_to_server(Commands::kill_ship, msg, getpid());
            Message is_killed(Commands::fail, "", -1);
            send(fdW, msg_to_server);
            recv(fdR, is_killed);
            if (is_killed._cmd == success) {
                std::cout << is_killed._data << std::endl;
                opponent.set(is_killed._data[0], is_killed._pid, '+');
            } else if (is_killed._cmd == end_game) {
                std::cout << is_killed._data << std::endl;
                return 1;
            } else if (is_killed._cmd == fail) {
                opponent.set(column[0], row, '*');
                std::cout << is_killed._data << std::endl;
            } else if (is_killed._cmd == disconnect) {
                return 0;
            } else {
                throw std::logic_error("unknown cmd");
            }
        }
    }
}

bool Client::loginToAccount(const std::string &login) {
    if (username != "") {
        std::cout << "\nYou are already loged in " + username << ' ' << '\n' << std::endl;
        return 0;
    }
    std::string reply;
    Message msg_to_server(Commands::login, login, getpid());
    Message reply_from_server(Commands::login, reply, 0);
    send(fdW, msg_to_server);
    recv(fdR, reply_from_server);
    if (reply_from_server._cmd == success) {
        std::cout << "\nAccout succesfully login in " << reply_from_server._data << std::endl;
        username = login;
        return 1;
    } else if (reply_from_server._cmd == fail) {
        std::cout << '\n' << reply_from_server._data << std::endl;
        return 0;
    } else {
        std::cout << '\n' << "Unknown command" << std::endl;
        return 0;
    }
}

bool Client::createAccount(const std::string &login) {
    std::string reply;
    Message msg_to_server(Commands::create_user, login, getpid());
    Message reply_from_server(create_user, reply, -1);
    send(fdW, msg_to_server);
    recv(fdR, reply_from_server);
    if (reply_from_server._cmd == success) {
        std::cout << "\nAccount succesfuly created " << reply_from_server._data << std::endl;
        return 1;
    } else if (reply_from_server._cmd == fail) {
        std::cout << "\nAccount is exist already " << reply_from_server._data << std::endl;
        return 0;
    } else {
        std::cout << "\nSomething get wrong" << std::endl;
        return 0;
    }
}

bool Client::getStats() {
    if (username == "") {
        std::cout << "\nYou are not authorized" << std::endl;
        return 0;
    }
    Message msg_to_server(Commands::stats, username, getpid());
    send(fdW, msg_to_server);
    Message reply_from_server(create_user, "", -1);
    recv(fdR, reply_from_server);
    if (reply_from_server._cmd == success) {
        std::cout << "\nStats of: " << username << std::endl;
        std::cout << "Win-Lose: " << reply_from_server._data << std::endl;
        return 1;
    } else if (reply_from_server._cmd == fail) {
        std::cout << "Shit happened " << reply_from_server._data << std::endl;
        return 0;
    } else {
        std::cout << "Shit happened " << reply_from_server._data << std::endl;
        return 0;
    }
}

bool Client::gameOperating(const std::string &login) {
    if (username == "") {
        std::cout << "You are not authorized" << std::endl;
        return 0;
    }
    if (username == login) {
        std::cout << "You can't play with yourself" << std::endl;
        return 0;
    }
    Message msg_to_server(Commands::find, login, getpid());
    send(fdW, msg_to_server);
    Message reply_from_server(fail, "", -1);
    recv(fdR, reply_from_server);
    if (reply_from_server._cmd == success) {
        Battlefield own_battlefield;
        Battlefield opponent_battlefield;
        if (!startGame(own_battlefield)) {
            std::cout << "Opponent disconnected" << std::endl;
            fsync(fdR);
            return 0;
        }
        recv(fdR, reply_from_server);
        if (reply_from_server._cmd == success) {
            if (!gameInProcces(own_battlefield, opponent_battlefield, reply_from_server._pid)) {
                std::cout << "Opponent disconnected" << std::endl;
                fsync(fdR);
                return 0;
            }
        }
    } else {
        std::cout << '\n' << reply_from_server._data << std::endl;
        return 1;
    }
}

void Client::exec() {
    std::string input, login;
    while (1) {
        std::cout << "Place for your command: ";
        std::cin >> input >> login;
        if (input == "login") {
            if (!loginToAccount(login)) {
                continue;
            }
        } else if (input == "create") {
            if (!createAccount(login)) {
                continue;
            }
        } else if (input == "stats") {
            if (!getStats()) {
                continue;
            }
        } else if (input == "find") {
            if (!gameOperating(login)) {
                continue;
            }
        }
        std::cout << std::endl;
    }
}
