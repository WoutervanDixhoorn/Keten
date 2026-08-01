#pragma once

#include "models/block.h"

#include <unordered_map>
#include <string>

namespace Keten {

	class Ledger {
	private:
		std::unordered_map<std::string, long> m_ledger;

	public:
		Ledger() = default;

		void ApplyBlock(const Block& block);
		long GetBalance(const std::string& publicKey) const;
	};

}