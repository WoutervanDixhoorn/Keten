#include "node.h"
#include "blockManager.h"

#include "network/messageFactory.h"

#include "utility/crypto.h"

#include <print>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <variant>

namespace Keten {

	Node::Node(const std::string& nodePort, const std::string& seedIp, const std::string& seedPort)
		: m_id(), m_transactionManager(m_id), m_blockManager(m_id, m_keten), m_network(nodePort, seedIp, seedPort), m_keten(), m_currentState(SyncingState())
	{
		Crypto::generateKeyPair(m_id.publicKey, m_id.privateKey);

		if (seedIp.empty() || seedPort.empty()) m_currentState = ActiveState();
	}

	void Node::Start(bool interactive) 
	{
		std::println("Start Keten Node...");

		m_network.Start();
		
		m_messageProcessingThread = std::jthread(&Node::processNetworkMessage, this);
		m_nodeProcessingThread = std::jthread(&Node::nodeProcessing, this);

		m_mainStateThread = std::jthread(&Node::runEventLoop, this);

		m_eventQueue.Push(NodeBeginSyncEvent());

		if(interactive) handleUserInput();
	}

	bool Node::SendTransaction(long amount, const std::string& receiverKey)
	{
		Transaction tx = m_transactionManager.CreateTransaction(amount, receiverKey);
		if (!m_transactionManager.ValidateTransaction(tx, m_ledger.GetBalance(tx.sender))) {
			return false;
		}
		m_transactionManager.AddTransaction(tx);

		NetworkMessage txMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::TRANSACTION, tx, NetworkMessageType::BROADCAST);
		m_network.PushMessage(txMsg);

		return true;
	}

	void Node::CreateGenesisBlock()
	{
		Block genBlock = m_blockManager.CreateGenesisBlock();
		NetworkMessage genBlockMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, genBlock, NetworkMessageType::BROADCAST);

		m_keten.AddAdmin(m_id.publicKey);
		m_keten.AddBlock(genBlock);
		m_ledger.ApplyBlock(genBlock);
		m_network.PushMessage(genBlockMessage);
	}

	void Node::SetState(NodeState state)
	{
		m_currentState = state;
	}

	uint32_t Node::GetChainHeight() const
	{
		return m_keten.Size();
	}

	bool Node::IsSynced() const
	{
		return std::holds_alternative<ActiveState>(m_currentState);
	}

	bool Node::IsDoneMinting() const
	{
		return m_transactionManager.IsEmpty();
	}

	long Node::GetBalance(const std::string& publicKey) const
	{
		return m_ledger.GetBalance(publicKey);
	}

	void Node::AddAdmin(const std::string& publicKey)
	{
		m_keten.AddAdmin(publicKey);
		m_consensus.AddAdmin(publicKey);
	}

	void Node::PushNetworkMessage(const NetworkMessage& message)
	{
		m_network.PushMessage(message);
	}

	void Node::PushEvent(const NodeEvent& event)
	{
		m_eventQueue.Push(event);
	}

	BlockManager& Node::GetBlockManager()
	{
		return m_blockManager;
	}

	TransactionManager& Node::GetTransactionManager()
	{
		return m_transactionManager;
	}

	Blockchain& Node::GetKeten()
	{
		return m_keten;
	}

	Ledger& Node::GetLedger()
	{
		return m_ledger;
	}

	const std::string& Node::GetPublicKey() const
	{
		return m_id.publicKey;
	}

	void Node::runEventLoop()
	{

		while (true)
		{
			NodeEvent event;
			if (!m_eventQueue.TryPop(event))
			{
				//TODO: Replace this with condition vairable
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			std::visit([this](auto& activeState, const auto& activeEvent) {
				activeState(activeEvent, *this);
			}, m_currentState, event);

		}

	}

	void Node::handleUserInput()
	{
		std::string input;
		while (true) 
		{
			std::getline(std::cin, input);

			if (input == "exit") 
			{
				m_network.Stop();
				break;
			}
			else if (input == "info") 
			{
				std::println("My Public Key: {}", m_id.publicKey);
			} else if (input.starts_with("send")) 
			{
				std::istringstream ss(input);

				std::string command;
				std::string receiverKey;
				long amount;

				if (ss >> command >> receiverKey >> amount) 
				{
					SendTransaction(amount, receiverKey);
				}
				else 
				{
					std::println("Invalid format! Use: send <key> <amount>");
					continue;
				}
			}
		}
	}

	void Node::processNetworkMessage()
	{
		while(true) 
		{

			NodeMessage message;
			if (!m_network.PollMessage(message)) 
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			switch (message.messageType)
			{

			case NodeMessageType::SYNC_REQUEST: {
				nlohmann::json payload = nlohmann::json::parse(message.payload);
				uint32_t height = payload["currentHeight"].get<uint32_t>();
				std::string requesterId = payload["nodeId"].get<std::string>();
				m_eventQueue.Push(NodeSyncRequestEvent{ .currentHeight = height, .requesterId = requesterId });
				break;
			}

			case NodeMessageType::SYNC_RESPONSE: {
				nlohmann::json payload = nlohmann::json::parse(message.payload);
				uint32_t targetHeight = payload["targetChainHeight"].get<uint32_t>();
				std::vector<Block> syncBlocks = payload["syncBlocks"].get<std::vector<Block>>();
				m_eventQueue.Push(NodeSyncResponseEvent{ .targetChainHeight = targetHeight, .syncBlocks = syncBlocks });
				break;
			}

			case NodeMessageType::TRANSACTION:
				m_eventQueue.Push(NodeTransactionEvent{ .tx = nlohmann::json::parse(message.payload).get<Transaction>() });
				break;

			case NodeMessageType::BLOCK:
				m_eventQueue.Push(NodeNewBlockEvent{ .newBlock = nlohmann::json::parse(message.payload).get<Block>() });
				break;

			}

		}
	}

	void Node::nodeProcessing()
	{
		auto timeSinceLastMintCheck = std::chrono::steady_clock::now();

		while (true)
		{
			size_t pendingCount = m_transactionManager.GetPendingTransactionCount();

			auto now = std::chrono::steady_clock::now();
			int elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - timeSinceLastMintCheck).count();
			
			bool sizeThresholdReached = pendingCount >= 10;
			bool timeThresholdReached = (pendingCount > 0 && elapsedSeconds >= 2);

			if (!sizeThresholdReached && !timeThresholdReached)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			auto nextToMint = m_consensus.GetNextToMintBlock(m_keten.Size());

			if (!nextToMint.has_value() || nextToMint.value().get() != m_id.publicKey)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			std::vector<Transaction> flushedTransactions = m_transactionManager.FlushPendingTransactions(10);
			Block newBlock = m_blockManager.MintBlock(m_keten.Size(), m_keten.GetLatestBlock().hash, flushedTransactions);
			m_keten.AddBlock(newBlock);
			m_ledger.ApplyBlock(newBlock);
			m_transactionManager.ApplyBlock(newBlock);

			NetworkMessage blockMsg = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, newBlock, NetworkMessageType::BROADCAST);
			m_network.PushMessage(blockMsg);
		}
	}
}