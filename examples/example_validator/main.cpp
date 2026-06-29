#include <print>

#include "keten/blockchain.h"
#include "keten/crypto.h"

typedef struct {
	std::string publicKey;
	std::string privateKey;
	Keten::Blockchain keten;
} KetenAdmin;

int main()
{
	KetenAdmin wouter;
	KetenAdmin kimy;
	Keten::generateKeyPair(wouter.publicKey, wouter.privateKey);
	Keten::generateKeyPair(kimy.publicKey, kimy.privateKey);

	wouter.keten.addAdmin(kimy.publicKey);
	wouter.keten.addAdmin(wouter.publicKey);
	kimy.keten.addAdmin(wouter.publicKey);
	kimy.keten.addAdmin(kimy.publicKey);

	//Is the genisisBlock
	Keten::Block lastBlock = wouter.keten.getLatestBlock();

	Keten::Transaction transaction;
	transaction.amount = 10;
	transaction.sender = wouter.publicKey;
	transaction.receiver = kimy.publicKey;

	Keten::Block block1(1, lastBlock.getHash());
	block1.addTransaction(transaction);

	std::string block1Hash = Keten::calculateHash(block1.getRawData());
	block1.setHash(block1Hash);
	std::string signature = Keten::signMessage(block1Hash, wouter.privateKey);
	block1.setSignature(signature);

	block1.setCreator(wouter.publicKey);

	kimy.keten.addBlock(block1);

	std::println("Success");

	return 0;
}