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

		Block(const nlohmann::json& j) {
			m_index = j.at("index").get<uint32_t>();
			m_prevHash = j.at("prevHash").get<std::string>();
			m_hash = j.at("hash").get<std::string>();
			m_creator = j.at("creator").get<std::string>();
			m_signature = j.at("signature").get<std::string>();

			for (const auto& txJson : j.at("transactions")) {
				m_transactions.push_back(Transaction(txJson));
			}
		}

		std::string toJson() const {
			json jString = {
				{"index", m_index},
				{"prevHash", m_prevHash},
				{"hash", m_hash},
				{"creator", m_creator},
				{"signature", m_signature},
				{"transactions", json::array()}
			};

			for (const auto& tx : m_transactions) {
				jString["transactions"].push_back(json::parse(tx.toJson()));
			}

			return jString.dump();
		}

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

			ss << m_index << m_prevHash << m_creator;
			for (const auto& tx : m_transactions) {
				ss << tx.txHash;
			}

			return ss.str();
		}

		void addTransaction(Transaction tx) {
			m_transactions.push_back(tx);
		}

		const std::vector<Transaction> getTransactions() const {
			return m_transactions;
		}

		const uint32_t getIndex() const {
			return m_index;
		}

		void setIndex(uint32_t index) {
			m_index = index;
		}

		void setSignature(const std::string signature) {
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