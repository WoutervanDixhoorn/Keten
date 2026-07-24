#pragma once

#include "blockchain.h"
#include "transactionManager.h"

#include "../interfaces/IMessageProcessor.h"
#include "../../network/messageTypes.h"
#include "../../network/P2PNetwork.h"

#include <print>

namespace Keten {

	class TransactionProcessor : public IMessageProcessor {
	private:
		std::shared_ptr<Blockchain> m_keten;
		std::shared_ptr<P2PNetwork> m_network;
		std::shared_ptr<TransactionManager> m_transactionManager;
	public:
		TransactionProcessor(std::shared_ptr<TransactionManager> txManager, std::shared_ptr<Blockchain> keten, std::shared_ptr<P2PNetwork> network) :
			m_keten(keten), m_network(network), m_transactionManager(txManager)
		{
		};

		virtual bool ProcessMessage(const MessageTypes& message) override {
			const NodeMessage* nodeMsg = std::get_if<NodeMessage>(&message);

			if (!nodeMsg) {
				return false;
			}

			const NodeMessage& incomingMsg = *nodeMsg;
			if (incomingMsg.messageType != NodeMessageType::TRANSACTION) return false;

			json msgJson = json::parse(incomingMsg.payload);
			Transaction incomingTransaction(msgJson);

			std::println("Message from {}\nAmount: {}", incomingTransaction.sender.substr(0, 6), incomingTransaction.amount);

			long senderBalance = m_keten->CalculateBalance(incomingTransaction.sender);
			if (senderBalance < incomingTransaction.amount) {
				std::println("Transaction REJECTED: Sender only has {} coins, tried to send {}!", senderBalance, incomingTransaction.amount);
				return false;
			}

			if (!m_transactionManager->ValidateTransaction(incomingTransaction)) {
				std::println("Transaction is not valid!");
				return false;
			}

			m_transactionManager->AddTransaction(incomingTransaction);

			std::println("Transaction is valid!");

			NetworkMessage netMessage = {
				.payload = incomingMsg.payload,
				.messageType = NetworkMessageType::BROADCAST
			};
			m_network->PushMessage(netMessage);

			return true;
		}
	};

}