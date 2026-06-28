#pragma once

#include <vector>

#include "picosha2.h"

#include "block.h"

namespace Keten {

	class Blockchain {
	public:
		Blockchain();

		bool addBlock(Block newBlock);
		const Block& getLatestBlock() const { return m_chain.back(); }

		void addAdmin(const std::string& adminPublicKey);

	private:
		std::vector<Block> m_chain;
		std::vector<std::string> m_adminKeys;

		void createGenesisBlock();
		bool isValidHash(const Block& b);
	};

}