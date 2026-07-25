#include "core/blockchain.h"

#include "models/transaction.h"
#include "models/block.h"

#include "utility/crypto.h"

#include <print>

struct KetenAdmin {
    std::string publicKey;
    std::string privateKey;
    Keten::Blockchain keten;
};

int main()
{
    std::println("=== CORE VALIDATION TEST ===");
    KetenAdmin wouter;
    KetenAdmin kimy;
    Keten::Crypto::generateKeyPair(wouter.publicKey, wouter.privateKey);
    Keten::Crypto::generateKeyPair(kimy.publicKey, kimy.privateKey);

    wouter.keten.AddAdmin(kimy.publicKey);
    wouter.keten.AddAdmin(wouter.publicKey);
    kimy.keten.AddAdmin(wouter.publicKey);
    kimy.keten.AddAdmin(kimy.publicKey);

    Keten::Block genesisBlock(0, "0000000000000000000000000000000000000000000000000000000000000000");
    genesisBlock.creator = wouter.publicKey;

    std::string genHash = Keten::Crypto::calculateHash(genesisBlock.getRawData());
    genesisBlock.hash = genHash;
    genesisBlock.signature = Keten::Crypto::signMessage(genHash, wouter.privateKey);

    wouter.keten.AddBlock(genesisBlock);
    kimy.keten.AddBlock(genesisBlock);

    Keten::Block lastBlock = wouter.keten.GetLatestBlock();

    Keten::Transaction transaction;
    transaction.amount = 10;
    transaction.sender = wouter.publicKey;
    transaction.receiver = kimy.publicKey;
    transaction.nonce = 1;
    transaction.txHash = Keten::Crypto::calculateHash(transaction.getRawData());
    transaction.signature = Keten::Crypto::signMessage(transaction.txHash, wouter.privateKey);

    Keten::Block block1(1, lastBlock.hash);
    block1.transactions.push_back(transaction);
    block1.creator = wouter.publicKey;

    std::string block1Hash = Keten::Crypto::calculateHash(block1.getRawData());
    block1.hash = block1Hash;
    block1.signature = Keten::Crypto::signMessage(block1Hash, wouter.privateKey);

    if (kimy.keten.AddBlock(block1)) {
        std::println("SUCCESS: Block successfully validated and added by Kimy.");
    }
    else {
        std::println("FAILED: Block was rejected.");
    }

    return 0;
}