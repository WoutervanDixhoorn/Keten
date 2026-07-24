#pragma once

#include "models/block.h"
#include "models/identity.h"

#include "network/messageTypes.h"

namespace Keten {

	class BlockManager {
	private:
		NodeIdentity& m_id;

	public:
		BlockManager(NodeIdentity& id) : m_id(id) {};

		Block CreateGenesisBlock();
		Block MintBlock(const uint32_t nextBlockIndex, const std::string prevBlockHash, std::vector<Transaction> transaction);
	};

}