#pragma once

#include "models/block.h"

#include <vector>
#include <mutex>

namespace Keten {

	class Blockchain {
	public:
		Blockchain() = default;

		long CalculateBalance(const std::string& publicKey);

		bool AddBlock(Block newBlock);
		const Block& GetLatestBlock() const;

		inline const size_t Size() const { return m_chain.size(); }

		void AddAdmin(const std::string& adminPublicKey);

	private:
		std::mutex m_chainMutex;

		std::vector<Block> m_chain;
		std::vector<std::string> m_adminKeys;

		bool isValidHash(const Block& b);
	};

}