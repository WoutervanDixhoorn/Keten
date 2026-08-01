#include "ledger.h"

namespace Keten {

	void Ledger::ApplyBlock(const Block& block)
	{
		const auto& transactions = block.transactions;

		for (const auto& tx : transactions)
		{
			m_ledger[tx.receiver] += tx.amount;
			m_ledger[tx.sender] -= tx.amount;
		}
	}

	long Ledger::GetBalance(const std::string& publicKey) const
	{
		if (m_ledger.find(publicKey) == m_ledger.end())
		{
			return 0;
		}

		return m_ledger.at(publicKey);
	}

}