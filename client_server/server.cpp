#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>
#include <sqlite3.h> 
#include <list>
#include <algorithm>

std::list<std::pair<int, std::string>> authorized_proccess;
std::string myfifo_read = "/tmp/myfifo_read";
std::string myfifo_write = "/tmp/myfifo_write";

std::pair<int, int> user_stats(const std::string& username) {
    sqlite3* db;
    if (sqlite3_open("../db/users.db", &db) == SQLITE_OK) {
        std::string query = "SELECT win, lose FROM users WHERE username=?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int win = sqlite3_column_int(stmt, 0);
                int lose = sqlite3_column_int(stmt, 1);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return std::make_pair(win, lose);
            } else {
                std::cerr << "User not found" << std::endl;
            }
        } else {
            std::cerr << "Error preparing statement" << std::endl;
        }
        sqlite3_close(db);
    } else {
        std::cerr << "Error opening database" << std::endl;
    }
    return std::make_pair(0, 0); // Return default values if error occurs
}

bool autorize(const std::string &username) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    
    rc = sqlite3_open("../db/users.db", &db);

    if (rc) {
        throw std::logic_error("can't open db");
    } else {
        std::string query = "SELECT COUNT(*) FROM users WHERE username='" + username + "';";
        int result;
        
        rc = sqlite3_exec(db, query.c_str(), [](void* data, int argc, char** argv, char** azColName) {
            int* count = static_cast<int*>(data);
            *count = atoi(argv[0]);
            return 0;
        }, &result, &zErrMsg);
        
        if (rc != SQLITE_OK) {
            throw std::logic_error("problem with db");
        }
        
        sqlite3_close(db);
        
        return result > 0;
    }
}

bool add_user(const std::string &username) {
    sqlite3 *db;
    int rc = sqlite3_open("../db/users.db", &db);
    if (rc) {
        std::cerr << "Ошибка при открытии базы данных: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    int status = 0;

    // Проверяем, существует ли запись с указанным username
    std::string selectQuery = "SELECT COUNT(*) FROM users WHERE username = ?";
    sqlite3_stmt *selectStmt;
    rc = sqlite3_prepare_v2(db, selectQuery.c_str(), -1, &selectStmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при подготовке SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_text(selectStmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(selectStmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(selectStmt, 0);
        if (count > 0) {
            // Запись существует
            status = -1;
        } else {
            // Запись не существует, выполняем вставку
            std::string insertQuery = "INSERT INTO users (username, win, lose) VALUES (?, ?, ?)";
            sqlite3_stmt *insertStmt;
            rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &insertStmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Ошибка при подготовке SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
                return false;
            }
            sqlite3_bind_text(insertStmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(insertStmt, 2, 0); // Значение win
            sqlite3_bind_int(insertStmt, 3, 0); // Значение lose
            
            rc = sqlite3_step(insertStmt);
            if (rc != SQLITE_DONE) {
                std::cerr << "Ошибка при выполнении SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return false;
            }
            
            // Запись успешно вставлена
            status = 1;
        }
    } else {
        std::cerr << "Ошибка при выполнении SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(selectStmt);
    sqlite3_close(db);

    return (status == 1);
}

bool is_authorized(int processId, const std::string& processName) {

    auto it = std::find(authorized_proccess.begin(), authorized_proccess.end(), std::make_pair(processId, processName));
    
    return (it != authorized_proccess.end());
}

 
int main() {

    int fd_read;
    int fd_write;
    mkfifo(myfifo_read.c_str(), 0666);
    mkfifo(myfifo_write.c_str(), 0666);
    int fdW = open(myfifo_read.c_str(), O_WRONLY);
    int fdR = open(myfifo_write.c_str(), O_RDONLY);
    Message msg_from_client;

    while (1) {
        size_t num_of_bytes = recv(fdR, msg_from_client);
        if (num_of_bytes == -1) {
            std::cout << "no data" << std::endl;
            continue;
        }
        if (msg_from_client._cmd == create_user) {
            bool success = add_user(msg_from_client._data);
            if (success) {
                Message msg_to_client(Commands::success, msg_from_client._data, -1);
                send(fdW, msg_to_client);   
            } else {
                Message msg_to_client(Commands::fail, msg_from_client._data, -1);
                send(fdW, msg_to_client);   
            }
        } else if (msg_from_client._cmd == Commands::login) {
            bool success = autorize(msg_from_client._data);
            if (success) {
                authorized_proccess.push_back(std::make_pair(msg_from_client._pid, std::string(msg_from_client._data)));
                Message msg_to_client(Commands::success, msg_from_client._data, -1);
                send(fdW, msg_to_client);   
            } else {
                Message msg_to_client(Commands::fail, msg_from_client._data, -1);
                send(fdW, msg_to_client);   
            }
            for (auto iter: authorized_proccess) {
                std::cout << iter.first << ' ' << iter.second << std::endl;
            }
        } else if (msg_from_client._cmd == Commands::stats) {
            std::cout << msg_from_client._data << ' ' << msg_from_client._pid << std::endl;
            if (is_authorized(msg_from_client._pid, std::string(msg_from_client._data))) {
                auto [win, lose] = user_stats(msg_from_client._data);
                Message msg_to_client(Commands::success, std::to_string(win) + ' ' + std::to_string(lose), -1);
                send(fdW, msg_to_client);
            } else {
                Message msg_to_client(Commands::fail, "not_autorized", -1);
                send(fdW, msg_to_client);
            }
        }
    }
    close(fdR);
    close(fdW);
    return 0;
}