#include "node.h"
#include "crypto.h"

#include <print>
#include <sstream>
#include <iostream>
#include <cstdint>

namespace Keten {

	Node::Node(const std::string nodePort, const std::string seedIp /* = ""*/, const std::string seedPort /*= ""*/)
		: m_network(nodePort, seedIp, seedPort)
	{
		generateKeyPair(m_id.publicKey, m_id.privateKey);
	
	}

	void Node::Start() {
		std::println("Start Keten Node...");

		m_network.Start();
		
		m_messageProcessingThread = std::jthread(&Node::processNetworkMessage, this);

		handleUserInput();
	}

	void Node::handleUserInput() {
		std::string input;
		while (true) {
			std::getline(std::cin, input);

			if (input == "exit") {
				m_network.Stop();
				break;
			}
			else if (input == "info") {
				std::println("My Public Key: {}", m_id.publicKey);
			} else if (input.starts_with("send")) {
				std::istringstream ss(input);

				std::string command;
				std::string receiverKey;
				double amount;

				if (ss >> command >> receiverKey >> amount) {
					std::println("Drawfting transaction of {} coins to {}...", amount, receiverKey.substr(0, 6));
				}
				else {
					std::println("Invalid format! Use: send <key> <amount>");
					continue;
				}

				Keten::Transaction transaction;
				transaction.amount = amount;
				transaction.sender = m_id.publicKey;
				transaction.receiver = receiverKey;

				std::string transactionHash = calculateHash(transaction.getRawData());
				std::string signature = signMessage(transactionHash, m_id.privateKey);
				transaction.signature = signature;

				m_pendingTransactions.push_back(transaction);

				NetworkMessage txMsg = {
					.payload = transaction.toJson(),
					.messageType = NetworkMessageType::DIRECT
				};

				m_network.PushMessage(txMsg);
			}
		}
	}

	void Node::processNetworkMessage() {
		while(true) {

			NodeMessage message;
			if (!m_network.PollMessage(message)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10)); //NOTE: Wait for message
				continue;
			}

			switch (message.messageType) {
				case NodeMessageType::TRANSACTION:
				default:
					if (processIncomingTransaction(message.payload)) {
						NetworkMessage netMessage = {
							.payload = message.payload,
							.messageType = NetworkMessageType::BOARDCAST
						};
						m_network.PushMessage(netMessage);
					}
					break;
			}

		}
	}

	bool Node::processIncomingTransaction(std::string transaction)
	{
		json msgJson = json::parse(transaction);
		Transaction incomingTransaction(msgJson);
	
		std::println("Message from {}\nAmount: {}", incomingTransaction.sender.substr(0, 6), incomingTransaction.amount);
		
		std::string transactionHash = calculateHash(incomingTransaction.getRawData());

		if (std::find(m_pendingTransactions.begin(), m_pendingTransactions.end(), transactionHash) != m_pendingTransactions.end()) 
		{
			std::println("Transaction {} already exists", transactionHash.substr(0, 6));
			return false;
		}
		incomingTransaction.txHash = transactionHash;

		std::string sender = incomingTransaction.sender;
		std::string signature = incomingTransaction.signature;

		if (!validateSignature(transactionHash, signature, sender)) {
			std::println("Transaction is not valid!");
			return false;
		}

		m_pendingTransactions.push_back(incomingTransaction);

		std::println("Transaction is valid!");

		return true;
	}

}