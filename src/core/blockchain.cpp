#include "blockchain.h"
#include "utility/crypto.h"

#include <print>
#include <algorithm>

namespace Keten {

	bool Blockchain::AddBlock(Block newBlock) 
	{
		if (m_chain.size() > 0 && newBlock.prevHash != m_chain.back().hash) 
		{
			//std::println("REJECTED: Chain link broken.");
			return false;
		}

		if (!isValidHash(newBlock)) 
		{
			std::println("REJECTED: Hash does not match data.");
			return false;
		}
	
		std::string creatorPublicKey = newBlock.creator;
		if (std::find(m_adminKeys.begin(), m_adminKeys.end(), creatorPublicKey) == m_adminKeys.end()) 
		{
			std::println("REJECTED: Creator is not on the admin VIP list.");
			return false;
		}

		if (!Crypto::validateSignature(newBlock.hash, newBlock.signature, creatorPublicKey))
		{
			std::println("REJECTED: Signature is invalid or forged.");
			return false;
		}

		std::lock_guard<std::mutex> guard(m_chainMutex);

		m_chain.push_back(newBlock);
		return true;
	}

	void Blockchain::AddAdmin(const std::string& adminPublicKey)
	{
		if (std::find(m_adminKeys.begin(), m_adminKeys.end(), adminPublicKey) == m_adminKeys.end())
		{
			m_adminKeys.push_back(adminPublicKey);
		}
	}

	long Blockchain::CalculateBalance(const std::string& publicKey) const
	{
		std::lock_guard<std::mutex> guard(m_chainMutex);

		long balance = 0;

		for (const auto& block : m_chain) 
		{
			const auto& transactions = block.transactions;

			for (const auto& tx : transactions) 
			{
				if (tx.receiver == publicKey) balance += tx.amount;
				if (tx.sender == publicKey) balance -= tx.amount;
			}
		}

		return balance;
	}

	const Block& Blockchain::GetLatestBlock() const
	{
		if (m_chain.empty()) 
		{
			throw std::runtime_error("FATAL: Cannot get latest block, blockchain is entirely empty!");
		}
		return m_chain.back();
	}

	const size_t Blockchain::Size() const
	{
		return m_chain.size();
	}

	bool Blockchain::isValidHash(const Block& b) 
	{
		return b.hash == Crypto::calculateHash(b.getRawData());
	}

}