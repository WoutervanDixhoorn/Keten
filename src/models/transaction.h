#pragma once

#include <string>
#include <cstdint>

#include "json.hpp"

namespace Keten {

	struct Transaction {
		std::string sender;
		std::string receiver;
		long amount;
		std::string signature;
		std::string txHash;
		uint64_t nonce;

		const std::string getRawData() const {
			return sender + receiver + std::to_string(amount) + std::to_string(nonce);
		}

		bool operator==(const std::string& hash) const { return txHash == hash; }

	};

	inline void to_json(nlohmann::json& j, const Transaction& tx) {
		j = nlohmann::json{
			{"sender", tx.sender},
			{"receiver", tx.receiver},
			{"amount", tx.amount},
			{"signature", tx.signature},
			{"nonce", tx.nonce},
			{"txHash", tx.txHash}
		};
	}

	inline void from_json(const nlohmann::json& j, Transaction& tx) {
		j.at("sender").get_to(tx.sender);
		j.at("receiver").get_to(tx.receiver);
		j.at("amount").get_to(tx.amount);
		j.at("signature").get_to(tx.signature);
		j.at("nonce").get_to(tx.nonce);
		j.at("txHash").get_to(tx.txHash);
	}

}