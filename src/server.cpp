#include "server.h"

std::string myfifo_read_default = "/tmp/myfifo_c-s_def";
int active_game_counter = 1;

void Server::tokenize(std::string input, int &direction, std::string &column, int &row, int &type) {
    std::vector<std::string> res;
    std::istringstream is(input);
    std::string part;
    while (is >> part) {
        res.push_back(part);
    }
    if (res.size() == 2) {
        direction = 0;
        column = res[0];
        row = std::stoi(res[1]);
        type = 0;
    } else {
        direction = std::stoi(res[0]);
        column = res[1];
        row = std::stoi(res[2]);
        type = std::stoi(res[3]);
    }
}

void Server::updateResultInBD(const std::string& username, bool isWin) {
    sqlite3 *db;
    if (sqlite3_open("../db/users.db", &db) != SQLITE_OK) {
        std::cerr << "Ошибка при открытии базы данных: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    const char *sql = isWin ? "UPDATE users SET win = win + 1 WHERE username = ?;"
                            : "UPDATE users SET lose = lose + 1 WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка при подготовке SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Ошибка при привязке параметра к запросу: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Ошибка при выполнении SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

std::pair<int, int> Server::user_stats(const std::string& username) {
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

bool Server::autorize(const std::string &username) {
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

bool Server::add_user(const std::string &username) {
    sqlite3 *db;
    int rc = sqlite3_open("../db/users.db", &db);
    if (rc) {
        std::cerr << "Ошибка при открытии базы данных: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    int status = 0;
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
            status = -1;
        } else {
            std::string insertQuery = "INSERT INTO users (username, win, lose) VALUES (?, ?, ?)";
            sqlite3_stmt *insertStmt;
            rc = sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &insertStmt, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Ошибка при подготовке SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
                return false;
            }
            sqlite3_bind_text(insertStmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(insertStmt, 2, 0);
            sqlite3_bind_int(insertStmt, 3, 0);
            rc = sqlite3_step(insertStmt);
            if (rc != SQLITE_DONE) {
                std::cerr << "Ошибка при выполнении SQL-запроса: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return false;
            }
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

Game::Game(const int &pid1_tmp, const int &pid2_tmp, const std::string &username1_tmp, const std::string &username2_tmp) {
    pid1 = pid1_tmp; pid2 = pid2_tmp;
    username1 = username1_tmp;
    username2 = username2_tmp;
    btf1 = new Battlefield(); btf2 = new Battlefield();
}

User::User(const std::string username_tmp, const int &pid_tmp, const int &fdW_tmp, const int &game_index_tmp, const int &game_status_tmp, const int &fdR_tmp) {
    username = username_tmp;
    pid = pid_tmp;
    fdW = fdW_tmp; fdR = fdR_tmp;
    game_index = game_index_tmp;
    game_status = game_status_tmp;
}

Server::Server() {
    std::cout << "Initialize default channel" << std::endl;
    mkfifo(myfifo_read_default.c_str(), 0666);
    fd_default_read = open(myfifo_read_default.c_str(), O_RDONLY | O_NONBLOCK);
    _fdR.push_back(fd_default_read);
    std::cout << "Default channel initialized" << std::endl;
}

Server::~Server() {
    for (auto fd: _fdR) {
        close(fd);
    }
    for (auto pid: _users) {
        close(pid.second.fdW);
    }
}

auto Server::search_by_username(const std::string &username) {
    for (auto iter = _users.begin(); iter != _users.end(); iter++) {
        if (iter->second.username == username) {
            return iter;
        }
    }
    return _users.end();
}

bool Server::is_authorized_by_username(const std::string &username) {
    bool flag = false;
    for (auto user: _users) {
        if (user.second.username == username) {
            flag = true;
        }
    }
    return flag;
}

bool Server::is_authorized_by_pid(const int &pid) {
    auto search = _users.find(pid);
    return search != _users.end();
}

void Server::try_recv() {
    auto start = std::chrono::system_clock::now();
    for (auto user: _users) {
        if (kill(user.second.pid, 0) != 0) {
            Message msg_from_client(clear, "", user.second.pid);
            _msgs.push(msg_from_client);
        }
    }
    while(true) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count() > 100) {
            break;
        }
        for (auto fd: _fdR) {
            Message msg_from_client;
            int num_of_bytes = recv(fd, msg_from_client);
            if (num_of_bytes > 0) {
                _msgs.push(msg_from_client);
            }
        }
    }
}

void Server::send_to(int pid, Message &msg) {
    send(_users[pid].fdW, msg);
}
void Server::userCreatingRoutine(const Message &msg_for_making_new_user) {
    bool success = add_user(msg_for_making_new_user._data);
    if (success) {
        Message msg_to_client(Commands::success, msg_for_making_new_user._data, -1);
        send_to(msg_for_making_new_user._pid, msg_to_client);   
    } else {
        Message msg_to_client(Commands::fail, msg_for_making_new_user._data, -1);
        send_to(msg_for_making_new_user._pid, msg_to_client);      
    }
}

void Server::clearAllDependenciesOfUser(const Message &msg_for_clear) {
    std::cout << "proccess with pid = " << msg_for_clear._pid << " disconnect" << std::endl;
    auto game_of_disconnected = _active_games.find(_users[msg_for_clear._pid].game_index);
    if (game_of_disconnected != _active_games.end()) {
        if (game_of_disconnected->second.pid1 == msg_for_clear._pid) {
            _users[game_of_disconnected->second.pid2].game_status = 0;
            Message msg_to_client(disconnect, "", -1);
            send_to(game_of_disconnected->second.pid2, msg_to_client);
        } else if (game_of_disconnected->second.pid2 == msg_for_clear._pid) {
            _users[game_of_disconnected->second.pid1].game_status = 0;
            Message msg_to_client(disconnect, "", -1);
            send_to(game_of_disconnected->second.pid1, msg_to_client);
        }
        _active_games.erase(_users[msg_for_clear._pid].game_index);
    }
    close(_users[msg_for_clear._pid].fdW);
    close(_users[msg_for_clear._pid].fdR);
    _fdR.erase(std::find(_fdR.begin(), _fdR.end(), _users[msg_for_clear._pid].fdR));
    _users.erase(_users.find(msg_for_clear._pid));
}

void Server::CheckAvailableUsernameAndLoginAccount(const Message &msg_for_login) {
    bool not_available_login = is_authorized_by_username(msg_for_login._data); 
    if (not_available_login) {
        Message msg_to_client(Commands::fail, "This username is already logged", -1);
        send_to(msg_for_login._pid, msg_to_client);
        return;
    }
    bool success = autorize(msg_for_login._data);
    if (success) {
        _users[msg_for_login._pid].username = std::string(msg_for_login._data);
        Message msg_to_client(Commands::success, msg_for_login._data, -1);
        send_to(msg_for_login._pid, msg_to_client);      
    } else {
        Message msg_to_client(Commands::fail, "No account with this username", -1);
        send_to(msg_for_login._pid, msg_to_client);      
    }
    std::cout << "Login:\n";
    for (auto iter: _users) {
        std::cout << iter.second.username << ' ' << iter.second.pid << std::endl;
    }
}

void Server::sendToClientInfoAboutStats(const Message &msg_for_stats) {
    if (is_authorized_by_pid(msg_for_stats._pid)) { 
        auto [win, lose] = user_stats(msg_for_stats._data);
        Message msg_to_client(Commands::success, std::to_string(win) + ' ' + std::to_string(lose), -1);
        send_to(msg_for_stats._pid, msg_to_client);   
    } else {
        Message msg_to_client(Commands::fail, "not_autorized", -1);
        send_to(msg_for_stats._pid, msg_to_client);   
    }
}

void Server::makeConnectionBetweenServerAndClient(const Message &msg_for_connect) {
    std::cout << "Making connection pipe for proc " << msg_for_connect._pid << std::endl;
    std::string name_fifo_read = "/tmp/myfifo_c-s_" + std::to_string(msg_for_connect._pid);
    std::string name_fifo_write = "/tmp/myfifo_s-c_" + std::to_string(msg_for_connect._pid);
    mkfifo(name_fifo_read.c_str(), 0666);
    mkfifo(name_fifo_write.c_str(), 0666);
    int fd_write = open(name_fifo_write.c_str(), O_WRONLY);
    int fd_read = open(name_fifo_read.c_str(), O_RDONLY | O_NONBLOCK);
    _fdR.push_back(fd_read);
    User tmp_user{"", msg_for_connect._pid, fd_write, 0, 0, fd_read};
    _users.insert({msg_for_connect._pid, tmp_user});
    auto elem = _users.find(msg_for_connect._pid);
    Message msg_to_client(Commands::success, "Succesfully connected", -1);
    if (elem != _users.end()) {
        send_to(elem->second.pid, msg_to_client);
    }
}

void Server::findOpponentForUser(const Message &msg_for_find) {
    std::cout << "Find opponent for " << _users[msg_for_find._pid].username << std::endl;
    if (is_authorized_by_username(msg_for_find._data)) {
        if (search_by_username(std::string(msg_for_find._data))->second.game_status == 1) {
            Message msg_to_client1(Commands::success, "Succesfully connected1", -1);
            Message msg_to_client2(Commands::success, "Succesfully connected2", -1);
            auto *user1 = &_users[msg_for_find._pid];
            auto *user2 = &search_by_username(std::string(msg_for_find._data))->second;
            user1->game_status = 2;
            user2->game_status = 2;
            user1->game_index = active_game_counter;
            user2->game_index = active_game_counter;
            Game game(user1->pid, user2->pid, user1->username, user2->username);
            _active_games.insert({active_game_counter, game});
            active_game_counter++;
            send_to(user2->pid, msg_to_client2);
            send_to(user1->pid, msg_to_client1);
        } else {
            _users[msg_for_find._pid].game_status = 1;
        }
    } else {
        Message msg_to_client(Commands::fail, "Opponent not online", -1);
        send_to(msg_for_find._pid, msg_to_client);
    }
}

void Server::placeShipOnBattleField(const Message &msg_for_place_ship) {
    std::cout << msg_for_place_ship._data << std::endl;
    std::string column; int row; int type; int direction;
    tokenize(std::string(msg_for_place_ship._data), direction, column, row, type);
    auto *deduction_proccess = &_active_games[_users[msg_for_place_ship._pid].game_index];
    if (deduction_proccess->pid1 == msg_for_place_ship._pid) {
        std::cout << "Place on first" << std::endl;
        deduction_proccess->btf1->place_ship(column.c_str()[0], row, Direction(direction), ShipType(type));
        Message msg_to_client1(Commands::success, "Succesfully placed", -1);
        send_to(msg_for_place_ship._pid, msg_to_client1);
    } else if (deduction_proccess->pid2 == msg_for_place_ship._pid) {
        std::cout << "Place on second" << std::endl;
        deduction_proccess->btf2->place_ship(column.c_str()[0], row, Direction(direction), ShipType(type));
        Message msg_to_client1(Commands::success, "Succesfully placed", -1);
        send_to(msg_for_place_ship._pid, msg_to_client1);
    } else {
        std::cout << "Proccess was disconnected and it was last attempt to do smt" << std::endl;
        return;
    }
    deduction_proccess->btf1->print();
    deduction_proccess->btf2->print();
}

void Server::makeUsersStatusToGameActive(const Message &msg_for_change_status) {
    auto *deduction_proccess = &_active_games[_users[msg_for_change_status._pid].game_index];
    Message msg_to_client1(Commands::success, "Ready to play", 1);
    Message msg_to_client2(Commands::success, "Ready to play", 2);
    if (deduction_proccess->pid1 == msg_for_change_status._pid && _users[deduction_proccess->pid2].game_status == 3) {
        send_to(deduction_proccess->pid1, msg_to_client1);
        send_to(deduction_proccess->pid2, msg_to_client2);
    }
    if (deduction_proccess->pid2 == msg_for_change_status._pid && _users[deduction_proccess->pid1].game_status == 3) {
        send_to(deduction_proccess->pid1, msg_to_client1);
        send_to(deduction_proccess->pid2, msg_to_client2);
    }
    _users[msg_for_change_status._pid].game_status = 3;
}

void Server::attackConcreteUsername(Battlefield *btf, Game *deduction_proccess, const Message &msg_for_attack) {
    std::string column; int row; int type; int direction;
    tokenize(std::string(msg_for_attack._data), direction, column, row, type);
    bool success = btf->try_kill(column.c_str()[0], row);
    if (success) {
        if (btf->end_game_check()) {
            Message msg_to_client1(Commands::end_game, "You win", 1);
            Message msg_to_client2(Commands::end_game, "You lose", 2);
            updateResultInBD(deduction_proccess->username1, 1);
            updateResultInBD(deduction_proccess->username2, 0);
            _active_games.erase(_active_games.find(_users[msg_for_attack._pid].game_index));
            _users[deduction_proccess->pid2].game_status = 0;
            _users[deduction_proccess->pid1].game_status = 0;
            _users[deduction_proccess->pid2].game_index = 0;
            _users[deduction_proccess->pid1].game_index = 0;
            send_to(deduction_proccess->pid1, msg_to_client1);
            send_to(deduction_proccess->pid2, msg_to_client2);
        } else {
            Message msg_to_client1(Commands::success, "Catch", 1);
            Message msg_to_client2(Commands::success, column, row);
            send_to(deduction_proccess->pid1, msg_to_client1);
            send_to(deduction_proccess->pid2, msg_to_client2);
        }
    } else {
        Message msg_to_client1(Commands::fail, "Failed attempt", 1);
        Message msg_to_client2(Commands::fail, column, row);
        send_to(deduction_proccess->pid1, msg_to_client1);
        send_to(deduction_proccess->pid2, msg_to_client2);
    }
}

void Server::userAttackOpponentBattleField(const Message &msg_for_attack) {
    auto *deduction_proccess = &_active_games[_users[msg_for_attack._pid].game_index];
    if (deduction_proccess->pid1 == msg_for_attack._pid) {
        attackConcreteUsername(deduction_proccess->btf2, deduction_proccess, msg_for_attack);
    } else if (deduction_proccess->pid2 == msg_for_attack._pid) {
        attackConcreteUsername(deduction_proccess->btf1, deduction_proccess, msg_for_attack);
    } else {
        Message msg_to_client(Commands::disconnect, "", -1);
        send_to(msg_for_attack._pid, msg_to_client);
        std::cout << "Proccess was disconnected and it was last attempt to do smt" << std::endl;
        return;
    }
    deduction_proccess->btf1->print();
    deduction_proccess->btf2->print();
}

void Server::exec() {
    while (1) {
        this->try_recv();
        while (!_msgs.empty()) {
            Message msg_to_make;
            msg_to_make = _msgs.front();
            _msgs.pop();
            switch (msg_to_make._cmd) {
                case connect:
                    makeConnectionBetweenServerAndClient(msg_to_make);
                    break;
                case create_user:
                    userCreatingRoutine(msg_to_make);
                    break;
                case clear:
                    clearAllDependenciesOfUser(msg_to_make);
                    break;
                case login:
                    CheckAvailableUsernameAndLoginAccount(msg_to_make);
                    break;
                case stats:
                    sendToClientInfoAboutStats(msg_to_make);
                    break;
                case find:
                    findOpponentForUser(msg_to_make);
                    break;
                case place_ship:
                    placeShipOnBattleField(msg_to_make);
                    break;
                case ready_to_play:
                    makeUsersStatusToGameActive(msg_to_make);
                    break;
                case kill_ship:
                    userAttackOpponentBattleField(msg_to_make);
                    break;
            }
        }
    }
}