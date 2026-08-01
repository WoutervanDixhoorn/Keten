#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <string>

namespace Keten {

	class Consensus {
	private:
		std::vector<std::string> m_adminKeys;

	public:
		void AddAdmin(const std::string& adminKey);

		std::optional<std::reference_wrapper<const std::string>> GetNextToMintBlock(uint64_t chainHeight) const;
	};

}