#include "client.h"

int main(int argc, char* argv[]) {
    Client client(getpid());
    client.textArt();
    std::cout << "Welcome to menu! Follow instruction:\n\n";
    std::cout << "| [login *username*] to login in account |    | [create *username*] to create new account |    |";
    std::cout << " [find *opponent*] to find opponent |    | [stats 1] for print your account stats |\n" << std::endl;
    client.exec();
}