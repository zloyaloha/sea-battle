#pragma once
#include <array>
#include <iostream>
#include <unordered_map>
#include <vector>

#define BTF_SIZE 10
#define SIZE_ERROR 2
#define PLACEMENT_ERROR 3
#define AMOUNT_OF_SHIPS_ERROR 4

enum Direction {
    Horisontal = 0,
    Vertical = 1,
};

enum DirectionOfShip {
    left, right, up, down, left_right, up_down
};

enum ShipType {
    four_square = 4,
    three_square = 3,
    two_square = 2,
    one_square = 1
};

class Battlefield {
    private:
        std::array<std::array<char, BTF_SIZE>, BTF_SIZE> _btf;
        std::unordered_map<ShipType, int> ships_amount;
    public:
        Battlefield();
        void print();
        int place_ship(char x, int y, Direction dir, ShipType type);
        bool try_kill(char x, int y);
        int four_amount() { return ships_amount.find(four_square)->second;}
        int three_amount() { return ships_amount.find(three_square)->second;}
        int two_amount() { return ships_amount.find(two_square)->second;}
        int one_amount() { return ships_amount.find(one_square)->second;}
        bool end_game_check();
        void set(char column, int x, char mark);
};