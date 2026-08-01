#include "transactionManager.h"

#include "utility/crypto.h"

#include "json.hpp"

namespace Keten {

	Transaction TransactionManager::CreateTransaction(const long amount, const std::string& receiver) const
	{
		Keten::Transaction transaction;
		transaction.amount = amount;
		transaction.sender = m_id.publicKey;
		transaction.receiver = receiver;
		transaction.nonce = ++m_transactionNonce;

		std::string transactionHash = Crypto::calculateHash(transaction.getRawData());
		std::string signature = Crypto::signMessage(transactionHash, m_id.privateKey);
		transaction.signature = signature;
		transaction.txHash = transactionHash;

		return transaction;
	}

	bool TransactionManager::ValidateTransaction(Transaction tx, long senderBalance) const
	{
		std::string transactionHash = Crypto::calculateHash(tx.getRawData());

		// TODO: ThreadSafe Queue/Mempool
		{
			std::lock_guard<std::mutex> guard(m_mempoolMutex);

			if (std::find(m_mempool.begin(), m_mempool.end(), transactionHash) != m_mempool.end())
			{
				return false;
			}
		}

		long totalBalance = senderBalance + GetPendingBalance(tx.sender);
		if (totalBalance < tx.amount) return false;

		tx.txHash = transactionHash;
		std::string sender = tx.sender;
		std::string signature = tx.signature;

		if (!Crypto::validateSignature(transactionHash, signature, sender))
		{
			return false;
		}

		return true;
	}

	void TransactionManager::AddTransaction(const Transaction tx)
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		m_mempool.push_back(tx);
	}

	void TransactionManager::ApplyBlock(const Block& block)
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		for (const auto& blockTx : block.transactions) {
			auto it = std::remove_if(m_mempool.begin(), m_mempool.end(), [&blockTx](const Transaction& mempoolTx) {
				return mempoolTx.txHash == blockTx.txHash;
				});
			if (it != m_mempool.end()) {
				m_mempool.erase(it, m_mempool.end());
			}
		}
		m_mempool.shrink_to_fit();
	}

	bool TransactionManager::IsEmpty() const
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		return m_mempool.size() == 0;
	}

	long TransactionManager::GetPendingBalance(const std::string& publicKey) const
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		long balance = 0;

		for (const auto& tx : m_mempool)
		{
			if (tx.receiver == publicKey) balance += tx.amount;
			if (tx.sender == publicKey) balance -= tx.amount;
		}

		return balance;
	}

	const std::vector<Transaction> TransactionManager::FlushPendingTransactions(const size_t limit)
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		std::sort(m_mempool.begin(), m_mempool.end(), [](const Transaction& a, const Transaction& b) {
			if (a.nonce != b.nonce) return a.nonce < b.nonce;
			return a.txHash < b.txHash;
		});

		auto startIter = m_mempool.begin();

		size_t safeLimit = (std::min)(limit, m_mempool.size());
		auto endIter = m_mempool.begin() + safeLimit;

		std::vector<Transaction> transactions(startIter, endIter);

		m_mempool.erase(startIter, endIter);
		m_mempool.shrink_to_fit();

		return transactions;
	}

	const size_t TransactionManager::GetPendingTransactionCount()
	{
		std::lock_guard<std::mutex> guard(m_mempoolMutex);

		return m_mempool.size();
	}

}