#pragma once

#include "IMessageProcessor.h"
#include "network/messageTypes.h"
#include "network/P2PNetwork.h"
#include "core/blockchain.h"

#include <print>
#include <variant>

namespace Keten {

	class BlockProcessor : public IMessageProcessor {
	private:
		Blockchain& m_keten;
		P2PNetwork& m_network;

	public:
		BlockProcessor(Blockchain& keten, P2PNetwork& network) : 
			m_keten(keten), m_network(network)
		{}

		virtual bool ProcessMessage(const MessageTypes& message) override 
		{
			const NodeMessage* nodeMsg = std::get_if<NodeMessage>(&message);

			if (!nodeMsg) 
			{
				return false;
			}

			const NodeMessage& incomingMsg = *nodeMsg;
			if (incomingMsg.messageType != NodeMessageType::BLOCK) return false;

			Block incomingBlock = nlohmann::json::parse(incomingMsg.payload).get<Block>();

			//TODO: Write some sort of simple logger so we can log to file, have different log levels and disable it completely
			std::println("Message with Block from {} ID: {}", incomingBlock.creator.substr(0, 6), incomingBlock.hash.substr(0, 6));

			if (!m_keten.AddBlock(incomingBlock))
			{
				return false;
			}

			NetworkMessage netMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, incomingBlock, NetworkMessageType::BROADCAST);
			m_network.PushMessage(netMessage);

			return true;
		}
	};

}