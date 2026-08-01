#pragma once
#include "models/identity.h"
#include "models/transaction.h"
#include "core/blockchain.h"

#include "../network/messageTypes.h"

#include <cstdint>
#include <vector>
#include <mutex>

namespace Keten {

	class TransactionManager {
	private:
		const NodeIdentity& m_id;

		mutable uint64_t m_transactionNonce = 0;
		std::vector<Transaction> m_mempool;

		mutable std::mutex m_mempoolMutex;
	public:
		TransactionManager(const NodeIdentity& id) : m_id(id) {}

		Transaction CreateTransaction(const long amount, const std::string& receiver) const;
		bool ValidateTransaction(Transaction tx, long mintedBalance) const;
		void AddTransaction(const Transaction tx);
		void ApplyBlock(const Block& block);

		bool IsEmpty() const;

		long GetPendingBalance(const std::string& publicKey) const;

		const std::vector<Transaction> FlushPendingTransactions(const size_t limit);
		const size_t GetPendingTransactionCount();
	};

}