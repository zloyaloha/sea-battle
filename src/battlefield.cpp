#include "battlefield.h"

Battlefield::Battlefield() {
    for (auto &row : _btf) {
        for (auto &element : row) {
            element = '0';
        }
    }
    ships_amount = {
        {four_square, 0},
        {three_square, 0},
        {two_square, 0},
        {one_square, 0}
    };
}

void Battlefield::print() {
    char column = 'A';
    std::cout << "   ";
    for (auto i: _btf) {
        std::cout << column << ' ';
        column++;
    }
    std::cout << std::endl;
    int row = 1;
    for (auto i: _btf) {
        if (row >= 10) {
            std::cout << row << " ";
        } else {
            std::cout << row << "  ";
        }
        for (auto j: i) {
            std::cout << j << ' ';
        }
        row++;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

char intToChar(int number) {
    return '0' + number;
}

bool Battlefield::end_game_check() {
    for (auto i: _btf) {
        for (auto j: i) {
            if (j >= '1' && j <= '4') {
                return false;
            }
        }
    }
    return true;
}

int Battlefield::place_ship(char column_char, int row, Direction dir, ShipType size) {
    int column = column_char - 'A';
    row--;
    char char_ship = intToChar(int(size));
    if (size == four_square && ships_amount.find(four_square)->second == 1) {
        return AMOUNT_OF_SHIPS_ERROR;
    }
    if (size == three_square && ships_amount.find(three_square)->second == 2) {
        return AMOUNT_OF_SHIPS_ERROR;
    }
    if (size == two_square && ships_amount.find(two_square)->second == 3) {
        return AMOUNT_OF_SHIPS_ERROR;
    }
    if (size == one_square && ships_amount.find(one_square)->second == 4) {
        return AMOUNT_OF_SHIPS_ERROR;
    }
    if (column < 0 || row < 0 || row > BTF_SIZE || column > BTF_SIZE) {
        return SIZE_ERROR;
    }
    if (dir == Direction::Horisontal) {
        for (size_t i = column; i < column + size; ++i) {
            if (_btf[row][i] != '0') {
                return PLACEMENT_ERROR;
            }
        }
        if (column + size > BTF_SIZE) {
            return SIZE_ERROR;
        }
        ships_amount[size] = ships_amount[size] + 1;
        for (int i = column; i < column + size; i++) {
            _btf[row][i] = char_ship;
            if (i == column) {
                if (i - 1 >= 0) {
                    _btf[row][i - 1] = '#';
                }
            }
            if (i == column + size - 1) {
                if (i + 1 < BTF_SIZE) {
                    _btf[row][i + 1]  = '#';
                }
            }
            if (row + 1 < BTF_SIZE) {
                _btf[row + 1][i] = '#';
            }
            if (row - 1 >= 0) {
                _btf[row - 1][i] = '#';
            }
        }
        return 0;
    } else {
        for (int i = row; i < row + size; ++i) {
            if (_btf[i][column] != '0') {
                return PLACEMENT_ERROR;
            }
        }
        if (row + size > BTF_SIZE) {
            return SIZE_ERROR;
        }
        ships_amount[size]++;
        for (int i = row; i < row + size; ++i) {
            _btf[i][column] = char_ship;
            if (i == row) {
                if (i - 1 >= 0) {
                    _btf[i - 1][column] = '#';
                }
            }
            if (i == row + size - 1) {
                if (i + 1 < BTF_SIZE) {
                    _btf[i + 1][column]  = '#';
                }
            }
            if (column + 1 < BTF_SIZE) {
                _btf[i][column + 1] = '#';
            }
            if (column - 1 >= 0) {
                _btf[i][column - 1]  = '#';
            }
        }
        return 0;
    }
}

void Battlefield::set(char column, int tmp, char mark) {
    int y = column - 'A';
    int x = tmp;
    _btf[x][y] = mark;
}

bool Battlefield::try_kill(char column, int y) {
    int x = y - 1;
    y = column - 'A';
    if (_btf[x][y] >= '1' && _btf[x][y] <= '4') {
        _btf[x][y] = '+';
        return true;
    } else {
        _btf[x][y] = '*';
        return false;
    }
}