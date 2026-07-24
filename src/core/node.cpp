#include "node.h"
#include "blockManager.h"

#include "network/messageFactory.h"

#include "processors/blockProcessor.h"
#include "processors/transactionProcessor.h"

#include "utility/crypto.h"

#include <print>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <chrono>


namespace Keten {

	Node::Node(const std::string nodePort, const std::string seedIp /* = ""*/, const std::string seedPort /*= ""*/)
		: m_id(), m_transactionManager(m_id), m_blockManager(m_id), m_network(nodePort, seedIp, seedPort), m_keten()
	{
		generateKeyPair(m_id.publicKey, m_id.privateKey);

		m_messageTypeProcessorMap[NodeMessageType::TRANSACTION] = std::make_unique<TransactionProcessor>(m_transactionManager, m_keten, m_network);
		m_messageTypeProcessorMap[NodeMessageType::BLOCK] = std::make_unique<BlockProcessor>(m_keten, m_network);
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
		NetworkMessage genBlockMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, genBlock, NetworkMessageType::BROADCAST);

		m_keten.AddAdmin(m_id.publicKey);
		m_keten.AddBlock(genBlock);
		m_network.PushMessage(genBlockMessage);
	}

	uint32_t Node::GetChainHeight()
	{
		return m_keten.Size();
	}

	long Node::CalculateBalance(const std::string publicKey)
	{
		return m_keten.CalculateBalance(publicKey);
	}

	bool Node::SendTransaction(long amount, std::string receiver)
	{
		std::println("Drawfting transaction of {} coins to {}...", amount, receiver.substr(0, 6));

		Transaction tx = m_transactionManager.CreateTransaction(amount, receiver);
		NetworkMessage txMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::TRANSACTION, tx, NetworkMessageType::BROADCAST);
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
				std::println("My Public Key: {}", m_id.publicKey);
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
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			if (m_messageTypeProcessorMap.contains(message.messageType)) {
				m_messageTypeProcessorMap[message.messageType]->ProcessMessage(message);
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
			Block newBlock = m_blockManager.MintBlock(m_keten.Size(), m_keten.GetLatestBlock().hash, flushedTransactions);
			m_keten.AddBlock(newBlock);

			NetworkMessage blockMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, newBlock, NetworkMessageType::BROADCAST);
			m_network.PushMessage(blockMsg);
		}
	}
}