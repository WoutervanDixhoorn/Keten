#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Keten {

	struct Transaction {
		std::string sender;
		std::string reciever;
		int64_t amount;
	};

}