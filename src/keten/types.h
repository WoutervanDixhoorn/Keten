#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "json.hpp"
using json = nlohmann::json;

namespace Keten {

	struct Transaction {
		std::string sender;
		std::string receiver;
		long amount;
		std::string signature;
		std::string txHash;

		Transaction() = default;

		Transaction(const nlohmann::json& j) {
			sender = j.at("sender").get<std::string>();
			receiver = j.at("receiver").get<std::string>();
			amount = j.at("amount").get<double>();
			signature = j.at("signature").get<std::string>();
		}

		bool operator==(const std::string& hash) { return txHash == hash; }

		std::string toJson() const {
			json jString = {
				{"sender", sender},
				{"receiver", receiver},
				{"amount", amount},
				{"signature", signature}
			};

			return jString.dump();
		}

		std::string getRawData() const {
			return sender + receiver + std::to_string(amount);
		}
	};

}