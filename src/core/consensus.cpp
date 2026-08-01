#include "consensus.h"

#include <algorithm>

namespace Keten {

	void Consensus::AddAdmin(const std::string& adminKey)
	{
		m_adminKeys.push_back(adminKey);
		std::sort(m_adminKeys.begin(), m_adminKeys.end());
	}

	std::optional<std::reference_wrapper<const std::string>> Consensus::GetNextToMintBlock(uint64_t chainHeight) const
	{
		if(m_adminKeys.size() == 0) return std::nullopt;

		int index = static_cast<int>(chainHeight % m_adminKeys.size());
		return std::cref(m_adminKeys.at(index));
	}

}
