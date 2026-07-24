#include "keten.h"
#include <print>

int main(int argc, char* argv[]) {
    if (argc == 2) {
        std::string myPort = argv[1];
        std::println("Starting Seed Node on port {}...", myPort);

        Keten::Node seedNode(myPort);
        seedNode.Start(true); // Zet op true als je de commando's (send, info) wilt gebruiken!
    }
    else if (argc == 4) {
        std::string myPort = argv[1];
        std::string seedIp = argv[2];
        std::string seedPort = argv[3];

        std::println("Starting Peer Node on port {}, connecting to {}:{}...", myPort, seedIp, seedPort);

        Keten::Node myNode(myPort, seedIp, seedPort);
        myNode.Start(true);
    }
    else {
        std::println("Usage (Seed): keten <my_port>");
        std::println("Usage (Peer): keten <my_port> <seed_ip> <seed_port>");
    }

    return 0;
}