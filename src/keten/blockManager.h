#pragma once

#include "types.h"
#include "block.h"

#include "../network/messageTypes.h"

namespace Keten {

	class BlockManager {
	private:
		std::shared_ptr<NodeIdentity> m_id;

	public:
		BlockManager(std::shared_ptr<NodeIdentity> id) : m_id(id) {};

		Block CreateGenesisBlock();
		Block MintBlock(const uint32_t nextBlockIndex, const std::string prevBlockHash, std::vector<Transaction> transaction);
	};

}