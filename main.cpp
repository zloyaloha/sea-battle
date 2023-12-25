#include "battlefield.h"

int main() {
    Battlefield btf;
    btf.print();
    btf.place_ship('B',1, Horisontal, three_square);
    btf.print();
    btf.try_kill('B', 1);
    btf.print();
}