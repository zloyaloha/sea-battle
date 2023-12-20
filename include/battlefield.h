#pragma once
#include <array>
#include <iostream>
#define BTF_SIZE 10

enum Direction {
    Horisontal,
    Vertical
};

class Battlefield {
    private:
        std::array<std::array<int, BTF_SIZE>, BTF_SIZE> _btf;
    public:
        Battlefield() : _btf{0} {};
        void print();
        bool place_ship(int x, int y, Direction dir, int size);
        int try_kill(int x, int y);
};