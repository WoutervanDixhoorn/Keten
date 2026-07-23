#pragma once

#include "messageTypes.h"
#include <optional>

#include "json.hpp"
using json = nlohmann::json;

namespace Keten {

	class MessageFactory {
	public:

		static NetworkMessage CreateNetworkMessage(NodeMessageType nodeType, const std::string& jsonPayload, NetworkMessageType networkType)
		{
			json envelope;
			envelope["type"] = static_cast<int>(nodeType);
			envelope["data"] = json::parse(jsonPayload)  ;

			NetworkMessage msg = {
				envelope.dump() + '\n',
				networkType
			};

			return msg;
		}

		static std::optional<NodeMessage> ParseNetworkFrame(const std::string frame)
		{
			try {
				json parsed = json::parse(frame);

				if (!parsed.contains("type") || !parsed["type"].is_number_integer() || !parsed.contains("data")) {
					return std::nullopt;
				}

				int typeInt = parsed["type"];
				NodeMessageType msgType = static_cast<NodeMessageType>(typeInt);

				return NodeMessage{
					.payload = parsed["data"].dump(),
					.messageType = msgType
				};
			}
			catch (const std::exception& e) {
				return std::nullopt;
			}
		}
	};

}