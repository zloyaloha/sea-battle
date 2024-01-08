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
        bool startGame(Battlefield &btf);
        bool gameInProcces(Battlefield &own, Battlefield &opponent, int number);
        void textArt();
    private:
        std::string myfifo_read, myfifo_write;
        std::string username;
        int fdR, fdW;
};