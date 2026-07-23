#include "transactionManager.h"

#include "crypto.h"

#include "json.hpp"

namespace Keten {

	Transaction TransactionManager::CreateTransaction(long amount, std::string receiver)
	{
		Keten::Transaction transaction;
		transaction.amount = amount;
		transaction.sender = m_id->publicKey;
		transaction.receiver = receiver;
		transaction.nonce = ++m_transactionNonce;

		std::string transactionHash = calculateHash(transaction.getRawData());
		std::string signature = signMessage(transactionHash, m_id->privateKey);
		transaction.signature = signature;
		transaction.txHash = transactionHash;

		m_pendingTransactions.push_back(transaction);

		return transaction;
	}

	bool TransactionManager::ValidateTransaction(Transaction tx)
	{
		std::string transactionHash = calculateHash(tx.getRawData());

		if (std::find(m_pendingTransactions.begin(), m_pendingTransactions.end(), transactionHash) != m_pendingTransactions.end())
		{
			return false;
		}

		tx.txHash = transactionHash;
		std::string sender = tx.sender;
		std::string signature = tx.signature;

		if (!validateSignature(transactionHash, signature, sender)) 
		{
			return false;
		}

		return true;
	}

	void TransactionManager::AddTransaction(Transaction tx)
	{
		m_pendingTransactions.push_back(tx);
	}

	const std::vector<Transaction> TransactionManager::FlushPendingTransactions()
	{
		std::vector<Transaction> transactions = m_pendingTransactions;
		m_pendingTransactions.clear();
		return transactions;
	}

}