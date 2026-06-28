#include "blockchain.h"

#include <print>
#include <algorithm>

#include "crypto.h"

namespace Keten {

	Blockchain::Blockchain() {
		createGenesisBlock();
	}

	bool Blockchain::addBlock(Block newBlock) {
		if (newBlock.getPrevHash() != m_chain.back().getHash()) {
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
	
	void Blockchain::addAdmin(const std::string& adminPublicKey) {
		if (std::find(m_adminKeys.begin(), m_adminKeys.end(), adminPublicKey) == m_adminKeys.end()) {
			m_adminKeys.push_back(adminPublicKey);
		}
	}

	void Blockchain::createGenesisBlock() {
		Block genesis(0, "0");
		genesis.setHash("0000genesis");
		m_chain.push_back(genesis);
	}

	bool Blockchain::isValidHash(const Block& b) {
		return b.getHash() == calculateHash(b.getRawData());
	}

}