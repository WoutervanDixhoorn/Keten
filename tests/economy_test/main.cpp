#include "keten.h"
#include <print>
#include <thread>
#include <chrono>

int main() {
    std::println("=== KETEN P2P BROADCAST TEST ===");

    Keten::Node seedNode("4455");
    seedNode.Start(false);

    Keten::Node nodeOne("4456", "127.0.0.1", "4455");
    nodeOne.Start(false);
    nodeOne.AddAdmin(seedNode.GetPublicKey());

    Keten::Node nodeTwo("4457", "127.0.0.1", "4455");
    nodeTwo.Start(false);
    nodeTwo.AddAdmin(seedNode.GetPublicKey());

    std::println("Waiting for network handshake (3 seconds)...");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::println("\n--- Minting Genesis Block on Seed Node ---");
    seedNode.CreateGenesisBlock();

    std::println("Waiting for blocks to propagate over P2P...");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::println("\n=== NETWORK LEDGER VERIFICATION ===");

    long seedBalanceOnSeed = seedNode.CalculateBalance(seedNode.GetPublicKey());
    long seedBalanceOnNodeOne = nodeOne.CalculateBalance(seedNode.GetPublicKey());
    long seedBalanceOnNodeTwo = nodeTwo.CalculateBalance(seedNode.GetPublicKey());

    std::println("Seed Node's local view of its balance: {} coins", seedBalanceOnSeed);
    std::println("Node One's view of Seed's balance:     {} coins", seedBalanceOnNodeOne);
    std::println("Node Two's view of Seed's balance:     {} coins", seedBalanceOnNodeTwo);

    if (seedBalanceOnNodeOne == seedBalanceOnSeed && seedBalanceOnNodeTwo == seedBalanceOnSeed) {
        std::println("\nSUCCESS: Block Broadcasting is working perfectly!");
        std::println("SUCCESS: Decentralized state achieved.");
    }
    else {
        std::println("\nFAILED: Nodes did not sync");
    }

    return 0;
}