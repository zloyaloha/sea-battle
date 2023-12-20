#include "battlefield.h"


void Battlefield::print() {
    for (auto i: _btf) {
        for (auto j: i) {
            std::cout << j << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

bool Battlefield::place_ship(int x, int y, Direction dir, int size) {
    int truly_x = BTF_SIZE - x - 1, truly_y = y;
    
    if (truly_x > BTF_SIZE || y > BTF_SIZE || truly_x < 0 || truly_y < 0) {
        return false;
    }
    if (dir == Direction::Horisontal) {
        for (size_t i = y; i <= size; ++i) {
            if (_btf[truly_x][i] != 0) {
                return false;
            }
        }
        if (x + size > BTF_SIZE) {
            return false;
        }
        for (size_t i = y; i <= size; ++i) {
            _btf[truly_x][i] = 1;
        }
        return true;
    } else {
        for (size_t i = truly_x; i > size; --i) {
            if (_btf[i][y] != 0) {
                return false;
            }
        }
        if (y + size > BTF_SIZE) {
            return false;
        }
        for (size_t i = truly_x; i > size; --i) {
            _btf[i][y] = 1;
        }
        return true;
    }
}

int Battlefield::try_kill(int x, int y) {
    int truly_x = BTF_SIZE - x - 1, truly_y = y;
    if (_btf[truly_x][truly_y] == 1) {
        return 1;
    } else {
        return 0;
    }
}