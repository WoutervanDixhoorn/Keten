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
		Node(const std::string nodePort, const std::string seedIp = "", const std::string seedPort = "");
		~Node() = default;

		void Start(bool interactive = true);
		
		void CreateGenesisBlock();
		uint32_t GetChainHeight();
		long CalculateBalance(const std::string publicKey);

		//TODO: Move this somewhere else I guess? And we still need the mechanism to asign admin users that need to verify. probably generate key pairs for the admin, this is ok for now!
		void AddAdmin(const std::string publicKey) { m_keten.AddAdmin(publicKey); }
		inline const std::string& GetPublicKey() const { return m_id.publicKey; }

		bool SendTransaction(long amount, std::string receiver);
		
	private:
		void handleUserInput();
		void processNetworkMessage();
		void nodeProcessing();

	private:
		NodeIdentity m_id;
		TransactionManager m_transactionManager;
		BlockManager m_blockManager;

		P2PNetwork m_network;
		Blockchain m_keten;
		
		std::map<NodeMessageType, std::unique_ptr<IMessageProcessor>> m_messageTypeProcessorMap;

		std::jthread m_messageProcessingThread;
		std::jthread m_nodeProcessingThread;
	};

}