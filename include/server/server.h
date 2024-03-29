#pragma once

#include "battlefield.h"
#include "message.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <sqlite3.h> 
#include <list>
#include <algorithm>
#include <queue>
#include <vector>
#include <chrono>
#include <errno.h>
#include <sstream>
#include <sys/types.h>
#include <signal.h>

struct Game {
    int pid1, pid2;
    std::string username1;
    std::string username2;
    Battlefield *btf1, *btf2;
    Game() = default;
    Game(const int &pid1_tmp, const int &pid2_tmp, const std::string &username1_tmp, const std::string &username2_tmp);
};

struct User {
    std::string username;
    int pid;
    int fdW;
    int fdR;
    int game_index;
    int game_status;
    User() = default;
    User(const std::string username_tmp, const int &pid_tmp, const int &fdW_tmp, const int &game_index_tmp, const int &game_status_tmp, const int &fdR_tmp);
};

class Server {
    public:
        Server();
        ~Server();
        auto search_by_username(const std::string &username);
        bool is_authorized_by_username(const std::string &username);
        bool is_authorized_by_pid(const int &pid);
        void tokenize(std::string input, int &direction, std::string &column, int &row, int &type);

        void try_recv();
        void send_to(int pid, Message &msg);
        void exec();

        void updateResultInBD(const std::string& username, bool isWin);
        std::pair<int, int> user_stats(const std::string& username);
        bool autorize(const std::string &username);
        bool add_user(const std::string &username);

        void userCreatingRoutine(const Message &msg_for_making_new_user);
        void clearAllDependenciesOfUser(const Message &msg_for_clear);
        void CheckAvailableUsernameAndLoginAccount(const Message &msg_for_login);
        void sendToClientInfoAboutStats(const Message &msg_for_stats);
        void makeConnectionBetweenServerAndClient(const Message &msg_for_connect);
        void findOpponentForUser(const Message &msg_for_find);
        void placeShipOnBattleField(const Message &msg_for_place_ship);
        void makeUsersStatusToGameActive(const Message &msg_for_change_status);
        void attackConcreteUsername(Battlefield *btf, Game *deduction_game, const Message &msg_for_attack);
        void userAttackOpponentBattleField(const Message &msg_for_attack);
    private:
        std::vector<int> _fdR;
        std::unordered_map<int, User> _users;
        std::queue<Message> _msgs;
        std::unordered_map<int, Game> _active_games;
        int fd_default_read;
}; 