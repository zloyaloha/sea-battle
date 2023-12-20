#include "battlefield.h"

int main() {
    Battlefield btf;
    btf.print();
    btf.place_ship(1,1,Vertical,4);
    btf.place_ship(1,0,Horisontal,4);
    btf.print();
}