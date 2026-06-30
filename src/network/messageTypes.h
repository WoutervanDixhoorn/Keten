#pragma once

#include <string>

namespace Keten {

	enum NetworkMessageType {
		DIRECT,
		BOARDCAST
	};

	struct NetworkMessage {
		std::string payload;
		NetworkMessageType messageType;
		std::string directAddress;
	};

	enum NodeMessageType {
		TRANSACTION
	};

	struct NodeMessage {
		std::string payload;
		NodeMessageType messageType;
	};
}