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

void text_art() {
    std::cout << '\n' << '\n';
    std::cout << "\t\t                      #######################\n";
    std::cout << "\t\t                   #####                  ######\n";
    std::cout << "\t\t                ####                           ###\n";
    std::cout << "\t\t               ###                               ###\n";
    std::cout << "\t\t              ##                                   ##\n";
    std::cout << "\t\t             ##                                     ##\n";
    std::cout << "\t\t            ##                                       ##\n";
    std::cout << "\t\t           ##                                        ##\n";
    std::cout << "\t\t           ##  ##                                 ##  ##\n";
    std::cout << "\t\t           ##  ##                                 ##  ##\n";
    std::cout << "\t\t           ##  ##                                 ##  ##\n";
    std::cout << "\t\t           ##  ##                                ##   ##\n";
    std::cout << "\t\t            ##  ##                               ##  ##\n";
    std::cout << "\t\t            ##  ##        ###         ###        ##  ##\n";
    std::cout << "\t\t             ## ##  ##########       ##########  ## ## \n";
    std::cout << "\t\t              #### ############     ############ ####\n";
    std::cout << "\t\t               ###  ##########       ##########  ##\n";
    std::cout << "\t\t    ####        #   #########         #########   #        ####\n";
    std::cout << "\t\t   ##  ##      ##    ######     # #    #######    ##     ###  ##\n";
    std::cout << "\t\t   ##   ##     ##              ## ##              ##    ###   ##\n";
    std::cout << "\t\t  ##     ####   ##            ### ###            ##  #####     ##\n";
    std::cout << "\t\t##          ########         #### ###          ########         ###\n";
    std::cout << "\t\t##   ###        ########     #### ###      ########       ####  ###\n";
    std::cout << "\t\t ###########       #######    ##   ##    ######        ###########\n";
    std::cout << "\t\t           #####    ##   ##             ## # ##    #####\n";
    std::cout << "\t\t              #####  # # ###           ### ###  #####\n";
    std::cout << "\t\t                  ####  #  # # # # # # #  # #####\n";
    std::cout << "\t\t                    ##  ## # # # # # # ####  #\n";
    std::cout << "\t\t                  ####   # # # # # # # ###  ######\n";
    std::cout << "\t\t              #####  ##    ##### # #####    ##  #####\n";
    std::cout << "\t\t     ##########      ###                   ##       ##########\n";
    std::cout << "\t\t    ##             ######                #######             ##\n";
    std::cout << "\t\t     ##         ####    #####         #####    ####         ##\n";
    std::cout << "\t\t      ###    ####          #############          ####    ###\n";
    std::cout << "\t\t       ##   ##                                       ##   ##\n";
    std::cout << "\t\t       ##  ##                                         ##  ##\n";
    std::cout << "\t\t        ####                                           ####\n" << std::endl << '\n' << '\n';

    std::cout << "  /$$$$$$  /$$$$$$$$  /$$$$$$          /$$$$$$$   /$$$$$$  /$$$$$$$$/$$$$$$$$/$$       /$$$$$$$$\n";
    std::cout << " /$$__  $$| $$_____/ /$$__  $$        | $$__  $$ /$$__  $$|__  $$__/__  $$__/ $$      | $$_____/\n";
    std::cout << "| $$  \\__/| $$      | $$  \\ $$        | $$  \\ $$| $$  \\ $$   | $$     | $$  | $$      | $$\n";
    std::cout << "|  $$$$$$ | $$$$$   | $$$$$$$$ /$$$$$$| $$$$$$$ | $$$$$$$$   | $$     | $$  | $$      | $$$$$\n";
    std::cout << " \\____  $$| $$__/   | $$__  $$|______/| $$__  $$| $$__  $$   | $$     | $$  | $$      | $$__/\n";
    std::cout << " /$$  \\ $$| $$      | $$  | $$        | $$  \\ $$| $$  | $$   | $$     | $$  | $$      | $$\n";
    std::cout << "|  $$$$$$/| $$$$$$$$| $$  | $$        | $$$$$$$/| $$  | $$   | $$     | $$  | $$$$$$$$| $$$$$$$$\n";
    std::cout << " \\______/ |________/|__/  |__/        |_______/ |__/  |__/   |__/     |__/  |________/|________/\n" << std::endl << '\n' << '\n';

}

std::string myfifo_write_default = "/tmp/myfifo_c-s_def";

std::string myfifo_read = "/tmp/myfifo_s-c_" + std::to_string(getpid());
std::string myfifo_write = "/tmp/myfifo_c-s_" + std::to_string(getpid());

int fdR, fdW;
std::string username = "";

bool default_connection() {
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
        return true;
    } else {
        close(fdR);
        close(fdW);
        std::cout << "Problem with connection" << std::endl;
        return false;
    }
}

bool start_game(Battlefield &btf) {
    while (btf.one_amount() != 4 && btf.two_amount() != 3 && btf.three_amount() != 2 && btf.four_amount() != 1) {
        btf.print();
        int a,b,c,d;
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

bool operate_game(Battlefield &own, Battlefield &opponent, int number) {
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

int main(int argc, char* argv[]) {
    text_art();
    default_connection();
    std::cout << "Welcome to menu! Follow instruction:\n\n";
    std::cout << "| [login *username*] to login in account |    | [create *username*] to create new account |    |";
    std::cout << " [find *opponent*] to find opponent |    | [stats 1] for print your account stats |\n" << std::endl;
    std::string input, login;
    while (1) {
        std::cout << "Place for your command: ";
        std::cin >> input >> login;
        if (input == "login") {
            if (username != "") {
                std::cout << "\nYou are already loged in " + username << ' ' << '\n' << std::endl;
                continue;
            }
            std::string reply;
            Message msg_to_server(Commands::login, login, getpid());
            Message reply_from_server(Commands::login, reply, 0);
            send(fdW, msg_to_server);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "\nAccout succesfully login in " << reply_from_server._data << std::endl;
                username = login;
            } else if (reply_from_server._cmd == fail) {
                std::cout << '\n' << reply_from_server._data << std::endl;
            }
        } else if (input == "create") {
            std::string reply;
            Message msg_to_server(Commands::create_user, login, getpid());
            Message reply_from_server(create_user, reply, -1);
            send(fdW, msg_to_server);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                std::cout << "\nAccount succesfuly created " << reply_from_server._data << std::endl;
            } else if (reply_from_server._cmd == fail) {
                std::cout << "\nAccount is exist already " << reply_from_server._data << std::endl;
            }
        } else if (input == "stats") {
            if (username == "") {
                std::cout << "\nYou are not authorized" << std::endl;
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
                std::cout << "Shit happened " << reply_from_server._data << std::endl;
            }
        } else if (input == "find") {
            if (username == "") {
                std::cout << "You are not authorized" << std::endl;
                continue;
            }
            if (username == login) {
                std::cout << "You can't play with yourself" << std::endl;
                continue;
            }
            Message msg_to_server(Commands::find, login, getpid());
            send(fdW, msg_to_server);
            Message reply_from_server(fail, "", -1);
            recv(fdR, reply_from_server);
            if (reply_from_server._cmd == success) {
                Battlefield own_battlefield;
                Battlefield opponent_battlefield;
                if (!start_game(own_battlefield)) {
                    std::cout << "Opponent disconnected" << std::endl;
                    fsync(fdR);
                    continue;
                }
                recv(fdR, reply_from_server);
                if (reply_from_server._cmd == success) {
                    if (!operate_game(own_battlefield, opponent_battlefield, reply_from_server._pid)) {
                        std::cout << "Opponent disconnected" << std::endl;
                        fsync(fdR);
                        continue;
                    }
                }
            } else {
                std::cout << '\n' << reply_from_server._data << std::endl;
            }
        }
        std::cout << std::endl;
    }
}