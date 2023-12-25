#include "battlefield.h"

int main() {
    Battlefield btf;
    btf.print();
    // btf.place_ship('A', 1, Horisontal, three_square);
    btf.place_ship('B',3, Vertical, three_square);
    btf.print();

}