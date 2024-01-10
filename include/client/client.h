#pragma once

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

class Client {
    public:
        Client() = default;
        Client(int pid);
        void exec();
        bool loginToAccount(const std::string &login);
        bool createAccount(const std::string &login);
        bool getStats();
        bool gameOperating(const std::string &login);
        void textArt();
        bool isAuthorized();
        bool checkCorrectUsername(const std::string &username);
    private:
        std::string myfifo_read, myfifo_write;
        std::string username;
        const std::string myfifo_write_default = "/tmp/myfifo_c-s_def";
        int fdR, fdW;
};

class GameOperator {
    private:
        Battlefield own, opponent;
        int fdR, fdW;
    public:
        GameOperator() = default;
        GameOperator(int setFdR, int setFdW) : fdR(setFdR), fdW(setFdW) {};
        bool placeShipRoutine();
        bool userMoveRoutine();
        bool waitingOpponentRoutine();
        bool gamingRoutine(int num);
        void userChooseChunckForAttack(std::string &column, int &row);
};