#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>
#include <sqlite3.h> 

std::string myfifo = "/tmp/myfifo";

bool to_db(char *sql) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc  = sqlite3_open("../db/users.db", &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    printf("data inserted\n");
    return 0;
}

int main() {
    int fd1;
    mkfifo(myfifo.c_str(), 0666);
    Message msg_from_client;
    while (1) {
        fd1 = open(myfifo.c_str(), O_RDONLY);
        size_t num_of_bytes = recv(fd1, msg_from_client);
        if (num_of_bytes == -1) {
            std::cout << "no data" << std::endl;
            continue;
        }
        if (msg_from_client._cmd = create_user) {
            std::string query = "INSERT INTO users (username, win, lose) VALUES('" + std::string(msg_from_client._data) + "', 0, 0)";
            std::cout << query << std::endl;
            char sql[46];
            strcpy(sql, query.c_str());
            to_db(sql);
        }
        close(fd1);
    }
    return 0;
}