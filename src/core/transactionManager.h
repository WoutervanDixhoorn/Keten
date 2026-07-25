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

		Transaction CreateTransaction(const long amount, const std::string& receiver);
		bool ValidateTransaction(Transaction tx) const;
		void AddTransaction(const Transaction tx);

		const std::vector<Transaction> FlushPendingTransactions();
		const size_t GetPendingTransactionCount();
	};

}