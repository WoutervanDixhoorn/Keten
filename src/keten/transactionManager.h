#pragma once

#include "types.h"
#include "../network/messageTypes.h"

namespace Keten {

	class TransactionManager {
	private:
		std::shared_ptr<NodeIdentity> m_id;
		
		uint64_t m_transactionNonce = 0;
		std::vector<Transaction> m_pendingTransactions;
	public:
		TransactionManager(std::shared_ptr<NodeIdentity> id) : m_id(id) {};

		Transaction CreateTransaction(long amount, std::string receiver);
		bool ValidateTransaction(Transaction tx);
		void AddTransaction(Transaction tx);

		const std::vector<Transaction> FlushPendingTransactions();
		inline size_t GetPendingTransactionCount() { return m_pendingTransactions.size(); }
	};

}