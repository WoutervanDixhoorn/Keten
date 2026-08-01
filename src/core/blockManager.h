#pragma once

#include "models/identity.h"
#include "core/blockchain.h"

#include "network/messageTypes.h"

namespace Keten {

	class BlockManager {
	private:
		const NodeIdentity& m_id;
		Blockchain& m_keten;

	public:
		BlockManager(const NodeIdentity& id, Blockchain& keten) : m_id(id), m_keten(keten) {};

		const Block CreateGenesisBlock() const;
		const Block MintBlock(const uint32_t nextBlockIndex, const std::string& prevBlockHash, std::vector<Transaction>& transaction) const;

		bool ProcessSync(const std::vector<Block>& blocks);
	};

}