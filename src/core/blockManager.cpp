#include "blockManager.h"

#include "models/block.h"
#include "utility/crypto.h"

namespace Keten {

	Block BlockManager::CreateGenesisBlock() 
	{
		Keten::Block genesisBlock(0, "0000000000000000000000000000000000000000000000000000000000000000");
		genesisBlock.creator = m_id.publicKey;

		Keten::Transaction transaction;
		transaction.amount = 1000000;
		transaction.sender = "SYSTEM";
		transaction.receiver = m_id.publicKey;
		transaction.nonce = 0;

		std::string transactionHash = calculateHash(transaction.getRawData());
		std::string signature = signMessage(transactionHash, m_id.privateKey);
		transaction.signature = signature;
		transaction.txHash = transactionHash;

		genesisBlock.transactions.push_back(transaction);

		std::string blockHash = calculateHash(genesisBlock.getRawData());
		genesisBlock.hash = blockHash;
		genesisBlock.signature = signMessage(blockHash, m_id.privateKey);

		return genesisBlock;
	}

	Block BlockManager::MintBlock(const uint32_t nextBlockIndex, const std::string prevBlockHash, std::vector<Transaction> transactions)
	{
		//NOTE: Strip away genesis hash into seperate place
		std::string prevHash = (nextBlockIndex == 0)
			? "0000000000000000000000000000000000000000000000000000000000000000"
			: prevBlockHash;

		Block newBlock(nextBlockIndex, prevHash);
		newBlock.creator = m_id.publicKey;
		/*for (const auto& tx : transactions) {
			newBlock.transactions.push_back(tx);
		}*/
		newBlock.transactions.append_range(transactions);

		std::string blockHash = calculateHash(newBlock.getRawData());
		newBlock.hash = blockHash;

		std::string signature = signMessage(blockHash, m_id.privateKey);
		newBlock.signature = signature;

		return newBlock;
	}

}