#pragma once
#include "models/identity.h"
#include "blockchain.h"
#include "transactionManager.h"
#include "blockManager.h"
#include "consensus.h"
#include "ledger.h"
#include "utility/threadSafeQueue.h"

#include "nodeState.h"

#include "../network/P2PNetwork.h"

#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <map>
#include <memory>
#include <variant>

namespace Keten {
	using NodeState = std::variant<SyncingState, ActiveState>;
	using NodeEvent = std::variant<NodeNewBlockEvent, NodeTransactionEvent, NodeBeginSyncEvent, NodeSyncRequestEvent, NodeSyncResponseEvent>;

	class Node {
	public:
		Node(const std::string& nodePort, const std::string& seedIp = "", const std::string& seedPort = "");
		~Node() = default;

		void Start(bool interactive = true);
		bool SendTransaction(long amount, const std::string& receiverKey);

	public:
		void CreateGenesisBlock();
		
		void SetState(NodeState state);
		void AddAdmin(const std::string& publicKey);

		void PushNetworkMessage(const NetworkMessage& message);
		void PushEvent(const NodeEvent& event);

		BlockManager& GetBlockManager();
		TransactionManager& GetTransactionManager();
		Blockchain& GetKeten();
		Ledger& GetLedger();

		const std::string& GetPublicKey() const;
		long GetBalance(const std::string& publicKey) const;
		uint32_t GetChainHeight() const;

		bool IsSynced() const;
		bool IsDoneMinting() const;

	private:
		void runEventLoop();
		
		void handleUserInput();
		void processNetworkMessage();
		void nodeProcessing();

	private:
		ThreadSafeQueue<NodeEvent> m_eventQueue;
		NodeState m_currentState;

		NodeIdentity m_id;
		P2PNetwork m_network;
		Blockchain m_keten;
		Consensus m_consensus;
		Ledger m_ledger;

		TransactionManager m_transactionManager;
		BlockManager m_blockManager;

		std::jthread m_messageProcessingThread;
		std::jthread m_nodeProcessingThread;
		std::jthread m_mainStateThread;
	};

}