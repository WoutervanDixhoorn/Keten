#include "keten.h"

#include <thread>
#include <print>
#include <chrono>
#include <string>
#include <vector>

// ==========================================
// === HELPER FUNCTIONS ===
// ==========================================

void SpamTransactions(Keten::Node& sender, const std::string& receiverKey, int count, int amount)
{
    std::println("Node {} is spamming {} transactions...", sender.GetPublicKey().substr(0, 6), count);
    for (int i = 0; i < count; i++) {
        sender.SendTransaction(amount, receiverKey);
    }
}

void SyncAdmins(const std::vector<Keten::Node*>& nodes, const std::vector<std::string>& adminKeys)
{
    for (auto* node : nodes) {
        for (const auto& key : adminKeys) {
            node->AddAdmin(key);
        }
    }
}

bool VerifyNetworkConsensus(const std::vector<Keten::Node*>& nodes, const std::vector<std::string>& trackedKeys)
{
    std::println("\n=== VALIDATING CONSENSUS ACROSS {} NODES ===", nodes.size());

    uint32_t baselineHeight = nodes[0]->GetChainHeight();
    bool heightSync = true;

    std::print("Chain Heights: ");
    for (size_t i = 0; i < nodes.size(); i++) {
        uint32_t h = nodes[i]->GetChainHeight();
        std::print("Node{}:{} ", i, h);
        if (h != baselineHeight) heightSync = false;
    }
    std::println("");

    if (!heightSync) {
        std::println("ERROR: Chain heights are out of sync!");
        return false;
    }

    bool balanceSync = true;
    for (const auto& key : trackedKeys) {
        long baselineBalance = nodes[0]->GetBalance(key);
        std::print("Balance for {}: ", key.substr(0, 6));

        for (size_t i = 0; i < nodes.size(); i++) {
            long bal = nodes[i]->GetBalance(key);
            std::print("Node{}:{} ", i, bal);
            if (bal != baselineBalance) balanceSync = false;
        }
        std::println("");
    }

    if (!balanceSync) {
        std::println("ERROR: Ledgers are out of sync!");
        return false;
    }

    std::println("SUCCESS: Perfect consensus reached across all active nodes.");
    return true;
}

// ==========================================
// === MAIN SYNC TEST ===
// ==========================================

int main()
{
    std::println("=== KETEN SYNCING LATE NODE TEST ===\n");

    std::println("[PHASE 1] Starting Seed Node...");
    Keten::Node seedNode("4455");
    seedNode.Start(false);

    std::vector<std::string> adminKeys = { seedNode.GetPublicKey() };
    SyncAdmins({ &seedNode }, adminKeys);

    std::println("Minting Genesis Block...");
    seedNode.CreateGenesisBlock();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::println("\n[PHASE 2] Growing chain to test 50-block batches...");
    SpamTransactions(seedNode, seedNode.GetPublicKey(), 600, 1);

    std::println("Waiting for Seed Node to mint all blocks...");
    while (!seedNode.IsDoneMinting()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::println("Seed Node Chain Height is now: {}", seedNode.GetChainHeight());

    std::println("\n[PHASE 3] Connecting Late Joiner Node...");
    Keten::Node lateJoiner("4456", "127.0.0.1", "4455");

    lateJoiner.Start(false);
    SyncAdmins({ &lateJoiner }, adminKeys);

    std::println("Waiting for Late Joiner to download and apply batches (5 seconds)...");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::vector<Keten::Node*> activeNodes = { &seedNode, &lateJoiner };
    std::vector<std::string> allKeys = { seedNode.GetPublicKey(), lateJoiner.GetPublicKey() };

    std::println("\n[PHASE 4] Validating Sync Success...");
    if (!VerifyNetworkConsensus(activeNodes, allKeys)) {
        return 1;
    }

    std::println("\nSUCCESS: Event-driven batch syncing is fully operational!");
    return 0;
}