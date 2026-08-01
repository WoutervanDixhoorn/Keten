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
	};

	enum NodeMessageType {
		TRANSACTION,
		BLOCK,

		SYNC_REQUEST,
		SYNC_RESPONSE,
	};

	struct NodeMessage {
		std::string payload;
		NodeMessageType messageType;
	};
}