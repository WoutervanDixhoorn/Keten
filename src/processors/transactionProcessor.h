#pragma once
#include "IMessageProcessor.h"

#include "core/blockchain.h"
#include "core/transactionManager.h"

#include "network/messageTypes.h"
#include "network/P2PNetwork.h"

#include <print>
#include <variant>

namespace Keten {

	class TransactionProcessor : public IMessageProcessor {
	private:
		Blockchain& m_keten;
		P2PNetwork& m_network;
		TransactionManager& m_transactionManager;
	public:
		TransactionProcessor(TransactionManager& txManager, Blockchain& keten, P2PNetwork& network) :
			m_keten(keten), m_network(network), m_transactionManager(txManager)
		{};

		virtual bool ProcessMessage(const MessageTypes& message) override {
			const NodeMessage* nodeMsg = std::get_if<NodeMessage>(&message);

			if (!nodeMsg) {
				return false;
			}

			const NodeMessage& incomingMsg = *nodeMsg;
			if (incomingMsg.messageType != NodeMessageType::TRANSACTION) return false;

			Transaction incomingTransaction = nlohmann::json::parse(incomingMsg.payload).get<Transaction>();

			std::println("Message from {}\nAmount: {}", incomingTransaction.sender.substr(0, 6), incomingTransaction.amount);

			long senderBalance = m_keten.CalculateBalance(incomingTransaction.sender);
			if (senderBalance < incomingTransaction.amount) {
				std::println("Transaction REJECTED: Sender only has {} coins, tried to send {}!", senderBalance, incomingTransaction.amount);
				return false;
			}

			if (!m_transactionManager.ValidateTransaction(incomingTransaction)) {
				std::println("Transaction is not valid!");
				return false;
			}

			m_transactionManager.AddTransaction(incomingTransaction);

			std::println("Transaction is valid!");

			NetworkMessage netMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::TRANSACTION, incomingTransaction, NetworkMessageType::BROADCAST);
			m_network.PushMessage(netMessage);

			return true;
		}
	};

}