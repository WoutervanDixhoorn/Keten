#pragma once

#include "models/block.h"

#include <vector>
#include <mutex>

namespace Keten {

	class Blockchain {
	public:
		Blockchain() = default;

		bool AddBlock(Block newBlock);
		void AddAdmin(const std::string& adminPublicKey);

	public:
		const long CalculateBalance(const std::string& publicKey) const;
		const Block& GetLatestBlock() const;
		const size_t Size() const;
	
	private:
		bool isValidHash(const Block& b);

	private:
		std::mutex m_chainMutex;

		std::vector<Block> m_chain;
		std::vector<std::string> m_adminKeys;
	};

}