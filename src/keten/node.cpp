#include "node.h"
#include "crypto.h"
#include "blockManager.h"
#include "../network/messageFactory.h"

#include <print>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <chrono>


namespace Keten {

	Node::Node(const std::string nodePort, const std::string seedIp /* = ""*/, const std::string seedPort /*= ""*/)
		: m_id(std::make_shared<NodeIdentity>()), m_transactionManager(m_id), m_blockManager(m_id), m_network(nodePort, seedIp, seedPort)
	{
		generateKeyPair(m_id->publicKey, m_id->privateKey);
	}

	void Node::Start(bool interactive) {
		std::println("Start Keten Node...");

		m_network.Start();
		
		m_messageProcessingThread = std::jthread(&Node::processNetworkMessage, this);
		m_nodeProcessingThread = std::jthread(&Node::nodeProcessing, this);

		if(interactive) handleUserInput();
	}

	void Node::CreateGenesisBlock()
	{
		Block genBlock = m_blockManager.CreateGenesisBlock();
		NetworkMessage genBlockMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, genBlock.toJson(), NetworkMessageType::BROADCAST);

		m_keten.AddAdmin(m_id->publicKey);
		m_keten.AddBlock(genBlock);
		m_network.PushMessage(genBlockMessage);
	}

	long Node::CalculateBalance(const std::string publicKey)
	{
		return m_keten.CalculateBalance(publicKey);
	}

	bool Node::SendTransaction(long amount, std::string receiver)
	{
		std::println("Drawfting transaction of {} coins to {}...", amount, receiver.substr(0, 6));

		Transaction tx = m_transactionManager.CreateTransaction(amount, receiver);
		NetworkMessage txMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::TRANSACTION, tx.toJson(), NetworkMessageType::BROADCAST);
		m_network.PushMessage(txMsg);

		return true;
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
				std::println("My Public Key: {}", m_id->publicKey);
			} else if (input.starts_with("send")) {
				std::istringstream ss(input);

				std::string command;
				std::string receiverKey;
				long amount;

				if (ss >> command >> receiverKey >> amount) {
					SendTransaction(amount, receiverKey);
				}
				else {
					std::println("Invalid format! Use: send <key> <amount>");
					continue;
				}
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
					if (processIncomingTransaction(message.payload)) {
						NetworkMessage netMessage = {
							.payload = message.payload,
							.messageType = NetworkMessageType::BROADCAST
						};
						m_network.PushMessage(netMessage);
					}
					break;
				case NodeMessageType::BLOCK:
					if (processIncomingBlock(message.payload)) {
						NetworkMessage netMessage = {
							.payload = message.payload,
							.messageType = NetworkMessageType::BROADCAST
						};
						m_network.PushMessage(netMessage);
					}
					break;
			}

		}
	}

	void Node::nodeProcessing()
	{
		while (true)
		{
			//NOTE: If a treshold is reached a block will get minted and spread over the network!
			if (m_transactionManager.GetPendingTransactionCount() < 10) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			std::vector<Transaction> flushedTransactions = m_transactionManager.FlushPendingTransactions();
			Block newBlock = m_blockManager.MintBlock(m_keten.Size(), m_keten.GetLatestBlock().getHash(), flushedTransactions);
			m_keten.AddBlock(newBlock);

			NetworkMessage blockMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, newBlock.toJson(), NetworkMessageType::BROADCAST);
			m_network.PushMessage(blockMsg);
		}
	}

	
	//TODO: MakeSeperate classes for handeling different message types that we can register!
	bool Node::processIncomingBlock(std::string block)
	{
		json msgJson = json::parse(block);
		Block incomingBlock(msgJson);

		std::println("Message with Block from {} ID: {}", incomingBlock.getCreator().substr(0, 6), incomingBlock.getHash().substr(0, 6));

		return m_keten.AddBlock(incomingBlock);
	}

	bool Node::processIncomingTransaction(std::string transaction)
	{
		json msgJson = json::parse(transaction);
		Transaction incomingTransaction(msgJson);
	
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

		return true;
	}

}