#pragma once
#include "models/identity.h"
#include "models/transaction.h"

#include "../network/messageTypes.h"

#include <cstdint>
#include <vector>
#include <mutex>

namespace Keten {

	class TransactionManager {
	private:
		NodeIdentity& m_id;
		
		uint64_t m_transactionNonce = 0;
		std::vector<Transaction> m_pendingTransactions;

		std::mutex m_pendingTransactionsMutex;
	public:
		TransactionManager(NodeIdentity& id) : m_id(id) {};

		Transaction CreateTransaction(long amount, std::string receiver);
		bool ValidateTransaction(Transaction tx);
		void AddTransaction(Transaction tx);

		const std::vector<Transaction> FlushPendingTransactions();
		inline size_t GetPendingTransactionCount() { return m_pendingTransactions.size(); }
	};

}