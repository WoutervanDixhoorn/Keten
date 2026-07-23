#include "blockchain.h"

#include <print>
#include <algorithm>

#include "crypto.h"

namespace Keten {

	bool Blockchain::AddBlock(Block newBlock) {
		if (m_chain.size() > 0 && newBlock.getPrevHash() != m_chain.back().getHash()) {
			std::println("REJECTED: Chain link broken.");
			return false;
		}

		if (!isValidHash(newBlock)) {
			std::println("REJECTED: Hash does not match data.");
			return false;
		}
	
		std::string creatorPublicKey = newBlock.getCreator();
		if (std::find(m_adminKeys.begin(), m_adminKeys.end(), creatorPublicKey) == m_adminKeys.end()) {
			std::println("REJECTED: Creator is not on the admin VIP list.");
			return false;
		}

		if (!validateSignature(newBlock.getHash(), newBlock.getSignature(), creatorPublicKey)) {
			std::println("REJECTED: Signature is invalid or forged.");
			return false;
		}

		m_chain.push_back(newBlock);
		return true;
	}
	
	long Blockchain::CalculateBalance(const std::string& publicKey)
	{
		long balance = 0;

		for (const auto& block : m_chain) {
			const auto& transactions = block.getTransactions();

			for (const auto& tx : transactions) {
				if (tx.receiver == publicKey) balance += tx.amount;
				if (tx.sender == publicKey) balance -= tx.amount;
			}
		}

		return balance;
	}

	void Blockchain::AddAdmin(const std::string& adminPublicKey) {
		if (std::find(m_adminKeys.begin(), m_adminKeys.end(), adminPublicKey) == m_adminKeys.end()) {
			m_adminKeys.push_back(adminPublicKey);
		}
	}

	bool Blockchain::isValidHash(const Block& b) {
		return b.getHash() == calculateHash(b.getRawData());
	}

}