#include "keten.h"
#include <thread>
#include <print>
#include <chrono>
#include <string>
#include <vector>
#include <memory>

#define TRANSACTION_COUNT 2000

// ==========================================
// === HELPER FUNCTIES ===
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
// === MAIN STRESS TEST ===
// ==========================================

int main()
{
    std::println("=== ADVANCED THROUGHPUT & P2P STRESS TEST ===\n");

    std::println("[FASE 1] Starting Core Network (Admins)...");
    auto seedNode = std::make_unique<Keten::Node>("4455");
    auto adminOne = std::make_unique<Keten::Node>("4456", "127.0.0.1", "4455");
    auto adminTwo = std::make_unique<Keten::Node>("4457", "127.0.0.1", "4455");

    seedNode->Start(false);

    std::vector<std::string> adminKeys = {
        seedNode->GetPublicKey(), adminOne->GetPublicKey(), adminTwo->GetPublicKey()
    };

    SyncAdmins({ seedNode.get(), adminOne.get(), adminTwo.get() }, adminKeys);

    std::println("\n[FASE 2] Introducing Non-Admin Node...");
    auto nonAdminNode = std::make_unique<Keten::Node>("4458", "127.0.0.1", "4455");
    nonAdminNode->Start(false);
    SyncAdmins({ nonAdminNode.get() }, adminKeys);

    seedNode->CreateGenesisBlock();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    adminOne->Start(false);
    adminTwo->Start(false);

    while (!adminOne->IsSynced() || !adminTwo->IsSynced()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::vector<Keten::Node*> activeNodes = { seedNode.get(), adminOne.get(), adminTwo.get(), nonAdminNode.get() };
    std::vector<std::string> allKeys = {
        seedNode->GetPublicKey(), adminOne->GetPublicKey(), adminTwo->GetPublicKey(), nonAdminNode->GetPublicKey()
    };

    VerifyNetworkConsensus(activeNodes, allKeys);

    std::println("\n[FASE 3] First Stress Wave...");
    SpamTransactions(*nonAdminNode, adminOne->GetPublicKey(), TRANSACTION_COUNT, 5);
    SpamTransactions(*seedNode, adminTwo->GetPublicKey(), TRANSACTION_COUNT, 10);

    while (nonAdminNode->GetChainHeight() < seedNode->GetChainHeight() ||
        !seedNode->IsDoneMinting() || !adminOne->IsDoneMinting() || !adminTwo->IsDoneMinting()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (!VerifyNetworkConsensus(activeNodes, allKeys)) return 1;

    std::println("\n[FASE 4] Introducing Late Joiner Node...");
    auto lateJoinerNode = std::make_unique<Keten::Node>("4459", "127.0.0.1", "4455");
    SyncAdmins({ lateJoinerNode.get() }, adminKeys);
    allKeys.push_back(lateJoinerNode->GetPublicKey());
    activeNodes.push_back(lateJoinerNode.get());
    lateJoinerNode->Start(false);

    while (!lateJoinerNode->IsSynced()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::println("\nLate joiner synced");
    if (!VerifyNetworkConsensus(activeNodes, allKeys)) return 1;

    std::println("\n[FASE 5] Second Stress Wave (Testing Late Joiner Sync)...");
    lateJoinerNode->SendTransaction(10, seedNode->GetPublicKey());
    SpamTransactions(*adminOne, lateJoinerNode->GetPublicKey(), 50, 100);

    std::println("Waiting for final block minting (8s)...");
    std::this_thread::sleep_for(std::chrono::seconds(8));

    std::println("\n[FASE 6] Final Global Consensus Validation...");
    if (!VerifyNetworkConsensus(activeNodes, allKeys)) return 1;

    std::println("\nSUCCESS: Network survived advanced stress test!");
    return 0;
}