#include "keten.h"
#include <thread>
#include <print>
#include <chrono>

#define TRANSACTION_COUNT 1000

int main() {
    std::println("=== THROUGHPUT & MINTING STRESS TEST ===");
    Keten::Node seedNode("4455");
    seedNode.Start(false);

    Keten::Node nodeOne("4456", "127.0.0.1", "4455");
    nodeOne.Start(false);

    Keten::Node nodeTwo("4457", "127.0.0.1", "4455");
    nodeTwo.Start(false);

    seedNode.AddAdmin(nodeOne.GetPublicKey());
    seedNode.AddAdmin(nodeTwo.GetPublicKey());

    nodeOne.AddAdmin(seedNode.GetPublicKey()); 
    nodeOne.AddAdmin(nodeOne.GetPublicKey());
    nodeOne.AddAdmin(nodeTwo.GetPublicKey());

    nodeTwo.AddAdmin(seedNode.GetPublicKey());
    nodeTwo.AddAdmin(nodeTwo.GetPublicKey());
    nodeTwo.AddAdmin(nodeOne.GetPublicKey());

    std::println("Waiting for network handshake (3 seconds)...");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    seedNode.CreateGenesisBlock();

    std::println("Waiting for genesis block to propagate...");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::println("SeedNode Balance: {}", seedNode.CalculateBalance(seedNode.GetPublicKey()));
    std::println("NodeOne Balance: {}", seedNode.CalculateBalance(nodeOne.GetPublicKey()));
    std::println("NodeTWo Balance: {}", seedNode.CalculateBalance(nodeTwo.GetPublicKey()));

    std::println("Nodes started. Firing {} transactions...", (TRANSACTION_COUNT * 3));

    for (int i = 0; i < TRANSACTION_COUNT; i++) {
        seedNode.SendTransaction(10, nodeTwo.GetPublicKey());
        nodeTwo.SendTransaction(5312, nodeOne.GetPublicKey());
    }

    for (int i = 0; i < TRANSACTION_COUNT; i++) {
        nodeOne.SendTransaction(1421, seedNode.GetPublicKey());
    }

    std::println("All transactions sent. Waiting for network to process and mint blocks...");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::println("SeedNode Balance: {}", seedNode.CalculateBalance(seedNode.GetPublicKey()));
    std::println("NodeOne Balance: {}", seedNode.CalculateBalance(nodeOne.GetPublicKey()));
    std::println("NodeTWo Balance: {}", seedNode.CalculateBalance(nodeTwo.GetPublicKey()));

    std::println("Seed Node Chain Height: {}", seedNode.GetChainHeight()); 
    std::println("SUCCESS: Stress test completed.");

    return 0;
}