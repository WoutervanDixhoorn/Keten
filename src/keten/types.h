#pragma once

#include <string>
#include <cstdint>
#include <sstream>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

namespace Keten {

	typedef struct {
		std::string publicKey;
		std::string privateKey;
	} NodeIdentity;

	struct Transaction {
		std::string sender;
		std::string receiver;
		long amount;
		std::string signature;
		std::string txHash;
		uint64_t nonce;

		Transaction() = default;

		Transaction(const nlohmann::json& j) {
			sender = j.at("sender").get<std::string>();
			receiver = j.at("receiver").get<std::string>();
			amount = j.at("amount").get<long>();
			signature = j.at("signature").get<std::string>();
			nonce = j.at("nonce").get<uint64_t>();
			txHash = j.at("txHash").get<std::string>();
		}

		bool operator==(const std::string& hash) { return txHash == hash; }

		std::string toJson() const {
			json jString = {
				{"sender", sender},
				{"receiver", receiver},
				{"amount", amount},
				{"signature", signature},
				{"nonce", nonce},
				{"txHash", txHash}
			};

			return jString.dump();
		}

		std::string getRawData() const {
			return sender + receiver + std::to_string(amount) + std::to_string(nonce);
		}
	};

}