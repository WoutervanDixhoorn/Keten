#pragma once
#include "models/identity.h"
#include "blockchain.h"
#include "transactionManager.h"
#include "blockManager.h"
#include "processors/IMessageProcessor.h"

#include "../network/P2PNetwork.h"

#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <map>
#include <memory>

namespace Keten {

	class Node {
	public:
		Node(const std::string& nodePort, const std::string& seedIp = "", const std::string& seedPort = "");
		~Node() = default;

		void Start(bool interactive = true);
		bool SendTransaction(long amount, std::string& receiver);
	public:
		void CreateGenesisBlock();
		void AddAdmin(const std::string& publicKey);

		const std::string& GetPublicKey() const;
		long CalculateBalance(const std::string& publicKey) const;
		uint32_t GetChainHeight() const;

	private:
		void handleUserInput();
		void processNetworkMessage();
		void nodeProcessing();

	private:
		NodeIdentity m_id;
		P2PNetwork m_network;
		Blockchain m_keten;

		TransactionManager m_transactionManager;
		BlockManager m_blockManager;

		std::map<NodeMessageType, std::unique_ptr<IMessageProcessor>> m_messageTypeProcessorMap;
		std::jthread m_messageProcessingThread;
		std::jthread m_nodeProcessingThread;
	};

}