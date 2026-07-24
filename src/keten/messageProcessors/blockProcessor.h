#pragma once

#include "blockchain.h"

#include "../interfaces/IMessageProcessor.h"
#include "../../network/messageTypes.h"
#include "../../network/P2PNetwork.h"

#include <print>

namespace Keten {

	class BlockProcessor : public IMessageProcessor {
	private:
		std::shared_ptr<Blockchain> m_keten;
		std::shared_ptr<P2PNetwork> m_network;
	public:
		BlockProcessor(std::shared_ptr<Blockchain> keten, std::shared_ptr<P2PNetwork> network) : 
			m_keten(keten), m_network(network)
		{};

		virtual bool ProcessMessage(const MessageTypes& message) override {
			const NodeMessage* nodeMsg = std::get_if<NodeMessage>(&message);

			if (!nodeMsg) {
				return false;
			}

			const NodeMessage& incomingMsg = *nodeMsg;
			if (incomingMsg.messageType != NodeMessageType::BLOCK) return false;

			json msgJson = json::parse(incomingMsg.payload);
			Block incomingBlock(msgJson);

			//TODO: Write some sort of simple logger so we can log to file, have different log levels and disable it completely
			std::println("Message with Block from {} ID: {}", incomingBlock.getCreator().substr(0, 6), incomingBlock.getHash().substr(0, 6));

			if (!m_keten->AddBlock(incomingBlock)) {
				return false;
			}

			NetworkMessage netMessage = {
				.payload = incomingMsg.payload,
				.messageType = NetworkMessageType::BROADCAST
			};
			m_network->PushMessage(netMessage);

			return true;
		}
	};

}