#pragma once

#include <vector>

#include "picosha2.h"

#include "block.h"

namespace Keten {

	class Blockchain {
	public:
		Blockchain() = default;

		long CalculateBalance(const std::string& publicKey);

		bool AddBlock(Block newBlock);
		inline const Block& GetLatestBlock() const { return m_chain.back(); }
		inline const size_t Size() const { return m_chain.size(); }

		void AddAdmin(const std::string& adminPublicKey);

	private:
		std::vector<Block> m_chain;
		std::vector<std::string> m_adminKeys;

		bool isValidHash(const Block& b);
	};

}