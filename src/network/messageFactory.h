#pragma once
#include "messageTypes.h"

#include "json.hpp"

#include <optional>

namespace Keten {

	class MessageFactory {
	public:

		template <typename T>
		static NetworkMessage CreateNetworkMessage(NodeMessageType nodeType, const T& payloadData, NetworkMessageType networkType)
		{
			nlohmann::json envelope;
			envelope["type"] = static_cast<int>(nodeType);
			envelope["data"] = payloadData;

			NetworkMessage msg = {
				envelope.dump() + '\n',
				networkType
			};

			return msg;
		}

		static std::optional<NodeMessage> ParseNetworkFrame(const std::string frame)
		{
			try 
			{
				nlohmann::json parsed = nlohmann::json::parse(frame);

				if (!parsed.contains("type") || !parsed["type"].is_number_integer() || !parsed.contains("data"))
				{
					return std::nullopt;
				}

				int typeInt = parsed["type"];
				NodeMessageType msgType = static_cast<NodeMessageType>(typeInt);

				return NodeMessage {
					.payload = parsed["data"].dump(),
					.messageType = msgType
				};
			}
			catch (const std::exception& e)
			{
				return std::nullopt;
			}
		}
	};

}