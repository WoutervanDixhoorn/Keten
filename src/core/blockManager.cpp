#include "blockManager.h"

#include "models/block.h"
#include "utility/crypto.h"

namespace Keten {

	const Block BlockManager::CreateGenesisBlock() const
	{
		Keten::Block genesisBlock(0, "0000000000000000000000000000000000000000000000000000000000000000");
		genesisBlock.creator = m_id.publicKey;

		Keten::Transaction transaction;
		transaction.amount = 1000000;
		transaction.sender = "SYSTEM";
		transaction.receiver = m_id.publicKey;
		transaction.nonce = 0;

		std::string transactionHash = Crypto::calculateHash(transaction.getRawData());
		std::string signature = Crypto::signMessage(transactionHash, m_id.privateKey);
		transaction.signature = signature;
		transaction.txHash = transactionHash;

		genesisBlock.transactions.push_back(transaction);

		std::string blockHash = Crypto::calculateHash(genesisBlock.getRawData());
		genesisBlock.hash = blockHash;
		genesisBlock.signature = Crypto::signMessage(blockHash, m_id.privateKey);

		return genesisBlock;
	}

	const Block BlockManager::MintBlock(const uint32_t nextBlockIndex, const std::string& prevBlockHash, std::vector<Transaction>& transactions) const
	{
		// TODO: Strip away genesis hash into seperate place
		std::string prevHash = (nextBlockIndex == 0)
			? "0000000000000000000000000000000000000000000000000000000000000000"
			: prevBlockHash;

		Block newBlock(nextBlockIndex, prevHash);
		newBlock.creator = m_id.publicKey;
		newBlock.transactions.append_range(transactions);

		std::string blockHash = Crypto::calculateHash(newBlock.getRawData());
		newBlock.hash = blockHash;

		std::string signature = Crypto::signMessage(blockHash, m_id.privateKey);
		newBlock.signature = signature;

		return newBlock;
	}

	bool BlockManager::ProcessSync(const std::vector<Block>& blocks)
	{
		for (auto& block : blocks)
		{
			if (!m_keten.AddBlock(block)) return false;
		}

		return true;
	}

}