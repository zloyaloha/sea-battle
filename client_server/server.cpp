#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "message.h"
#include <iostream>
#include <sqlite3.h> 
#include <list>
#include <algorithm>
#include <queue>
#include <vector>
#include <chrono>
#include <errno.h>

std::string myfifo_read_default = "/tmp/myfifo_c-s_def";

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

class Server {
    public:
        std::vector<int> _fdR;
        std::unordered_map<int, int> _pid_fdW;
        std::unordered_map<int, std::string> authorized_proccess;
        std::queue<Message> _msgs;
        int fd_default_read;
        Server() {
            std::cout << "Initialize default channel" << std::endl;
            mkfifo(myfifo_read_default.c_str(), 0666);
            fd_default_read = open(myfifo_read_default.c_str(), O_RDONLY | O_NONBLOCK);
            _fdR.push_back(fd_default_read);
            std::cout << "Default channel initialized" << std::endl;
        }
        ~Server() {
            for (auto fd: _fdR) {
                close(fd);
            }
            for (auto pid: _pid_fdW) {
                close(pid.second);
            }
            
        }
        bool is_authorized(int processId) {
            auto elem = authorized_proccess.find(processId);
            return (elem != authorized_proccess.end());
        }
        void try_recv() {
            auto start = std::chrono::system_clock::now();
            while(true) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > 100) {
                    break;
                }
                for (auto fd: _fdR) {
                    Message msg_from_client;
                    int num_of_bytes = recv(fd, msg_from_client);
                    // std::cout << "huynul v o4ered" << std::endl;
                    if (num_of_bytes > 0) {
                        _msgs.push(msg_from_client);
                    }
                }
            }
        }
        void send_to(int pid, Message &msg) {
            auto elem = _pid_fdW.find(pid);
            if (elem != _pid_fdW.end()) {
                send((*elem).second, msg);
            }
        }
        void exec() {
            while (1) {
                this->try_recv();
                while (!_msgs.empty()) {
                    Message msg_to_make;
                    msg_to_make = _msgs.back();
                    _msgs.pop();
                    if (msg_to_make._cmd == create_user) {
                        bool success = add_user(msg_to_make._data);
                        if (success) {
                            Message msg_to_client(Commands::success, msg_to_make._data, -1);
                            send_to(msg_to_make._pid, msg_to_client);   
                        } else {
                            Message msg_to_client(Commands::fail, msg_to_make._data, -1);
                            send_to(msg_to_make._pid, msg_to_client);      
                        }
                    } else if (msg_to_make._cmd == Commands::login) {
                        bool success = autorize(msg_to_make._data);
                        if (success) {
                            authorized_proccess.insert({msg_to_make._pid, std::string(msg_to_make._data)});
                            Message msg_to_client(Commands::success, msg_to_make._data, -1);
                            send_to(msg_to_make._pid, msg_to_client);      
                        } else {
                            Message msg_to_client(Commands::fail, msg_to_make._data, -1);
                            send_to(msg_to_make._pid, msg_to_client);      
                        }
                        for (auto iter: authorized_proccess) {
                            std::cout << iter.first << ' ' << iter.second << std::endl;
                        }
                    } else if (msg_to_make._cmd == Commands::stats) {
                        if (is_authorized(msg_to_make._pid)) {
                            auto [win, lose] = user_stats(msg_to_make._data);
                            Message msg_to_client(Commands::success, std::to_string(win) + ' ' + std::to_string(lose), -1);
                            send_to(msg_to_make._pid, msg_to_client);   
                        } else {
                            Message msg_to_client(Commands::fail, "not_autorized", -1);
                            send_to(msg_to_make._pid, msg_to_client);   
                        }
                    } else if (msg_to_make._cmd == Commands::connect) {
                        if (close(fd_default_read) == -1) {
                            throw std::logic_error("bad with close");
                        }
                        if (unlink(myfifo_read_default.c_str())) {
                            throw std::logic_error("bad with unlink");
                        }
                        mkfifo(myfifo_read_default.c_str(), 0666);
                        fd_default_read = open(myfifo_read_default.c_str(), O_RDONLY | O_NONBLOCK);

                        std::cout << "Making connection pipe for proc " << msg_to_make._pid << std::endl;
                        std::string name_fifo_read = "/tmp/myfifo_c-s_" + std::to_string(msg_to_make._pid);
                        std::string name_fifo_write = "/tmp/myfifo_s-c_" + std::to_string(msg_to_make._pid);
                        mkfifo(name_fifo_read.c_str(), 0666);
                        mkfifo(name_fifo_write.c_str(), 0666);
                        int fd_write= open(name_fifo_write.c_str(), O_WRONLY);
                        int fd_read = open(name_fifo_read.c_str(), O_RDONLY);

                        _fdR.push_back(fd_read);
                        _pid_fdW.insert({msg_to_make._pid, fd_write});
                        auto elem = _pid_fdW.find(msg_to_make._pid);
                        Message msg_to_client(Commands::success, "Succesfully connected", -1);
                        if (elem != _pid_fdW.end()) {
                            send((*elem).second, msg_to_client);
                        }
                    }
                }
            }
        }
};

int main() {
    Server server;
    server.exec();
}