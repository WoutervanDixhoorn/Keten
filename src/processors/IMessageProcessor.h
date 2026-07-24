#pragma once

#include "network/messageTypes.h"

#include <string>
#include <variant>

namespace Keten {
	
	using MessageTypes = std::variant<NodeMessage>;

	class IMessageProcessor {
	public:
		virtual ~IMessageProcessor() = default;

		virtual bool ProcessMessage(const MessageTypes& message) = 0;
	};

}