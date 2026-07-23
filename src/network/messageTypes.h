#pragma once

#include <string>

namespace Keten {

	enum NetworkMessageType {
		DIRECT,
		BROADCAST,
		
	};

	struct NetworkMessage {
		std::string payload;
		NetworkMessageType messageType;
		std::string directAddress;
	};

	enum NodeMessageType {
		TRANSACTION,
		BLOCK
	};

	struct NodeMessage {
		std::string payload;
		NodeMessageType messageType;
	};
}