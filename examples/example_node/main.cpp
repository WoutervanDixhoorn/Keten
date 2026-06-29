#include "keten.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 2) {
        std::string myPort = argv[1];
        Keten::Node seedNode(myPort);
        seedNode.Start();
    }

    else if (argc == 4) {
        std::string myPort = argv[1];
        std::string seedIp = argv[2];
        std::string seedPort = argv[3];

        Keten::Node myNode(myPort, seedIp, seedPort);
        myNode.Start();
    }
    else {
        std::cout << "Usage (Seed): keten <my_port>\n";
        std::cout << "Usage (Peer): keten <my_port> <seed_ip> <seed_port>\n";
    }

    return 0;
}