#pragma once
#include <memory>
#include <sstream>

#include "types.h"

namespace Keten {

	class Block {
	public:
		Block(uint32_t index, std::string prevHash)
			: m_index(index), m_prevHash(prevHash)
		{}

		void setHash(std::string hash) {
			m_hash = hash;
		}

		std::string getHash() const {
			return m_hash;
		}

		void setPrevHash(std::string hash) {
			m_prevHash = hash;
		}

		std::string getPrevHash() const {
			return m_prevHash;
		}

		std::string getRawData() const {
			std::stringstream ss;

			ss << m_index << m_prevHash;
			for (const auto& tx : m_transactions) {
				ss << tx.sender << tx.receiver << tx.amount;
			}

			return ss.str();
		}

		void addTransaction(Transaction tx) {
			m_transactions.push_back(tx);
		}

		void setSignature(const std::string& signature) {
			m_signature = signature;
		}

		std::string getSignature() const {
			return m_signature;
		}

		void setCreator(const std::string creatorPublicKey) {
			m_creator = creatorPublicKey;
		}

		std::string getCreator() const {
			return m_creator;
		}

	private:
		std::vector<Transaction> m_transactions;
		std::string m_signature;
		std::string m_creator;

		uint32_t m_index;
		std::string m_prevHash;
		std::string m_hash;
	};

}