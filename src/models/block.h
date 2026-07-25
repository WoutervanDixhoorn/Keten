#pragma once
#include "transaction.h"

#include "json.hpp"

#include <vector>
#include <memory>
#include <sstream>

namespace Keten {

	struct Block {
		uint32_t index;
		std::string prevHash;
		std::string hash;
		std::string creator;
		std::string signature;
		std::vector<Transaction> transactions;

		const std::string getRawData() const {
			std::stringstream ss;
			ss << index << prevHash << creator;
			for (const auto& tx : transactions) {
				ss << tx.txHash;
			}
			return ss.str();
		}
	};

	inline void to_json(nlohmann::json& j, const Block& block) {
		j = {
			{"index", block.index},
			{"prevHash", block.prevHash},
			{"hash", block.hash},
			{"creator", block.creator},
			{"signature", block.signature},
			{"transactions", block.transactions}
		};
	}

	inline void from_json(const nlohmann::json& j, Block& block) {
		j.at("index").get_to(block.index);
		j.at("prevHash").get_to(block.prevHash);
		j.at("hash").get_to(block.hash);
		j.at("creator").get_to(block.creator);
		j.at("signature").get_to(block.signature);
		j.at("transactions").get_to(block.transactions);
	}
}