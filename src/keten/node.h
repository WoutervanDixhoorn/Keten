#pragma once
#include "blockchain.h"
#include "types.h"

#include "../network/P2PNetwork.h"

#include <string>
#include <vector>

namespace Keten {

	typedef struct {
		std::string publicKey;
		std::string privateKey;
	} NodeIdentity;

	class Node {
	public:
		Node(const std::string nodePort, const std::string seedIp = "", const std::string seedPort = "");
		~Node() = default;

		void Start();

	private:
		void handleUserInput();
		void processNetworkMessage();
		bool processIncomingTransaction(std::string transaction);

	private:
		NodeIdentity m_id;
		P2PNetwork m_network;
		Blockchain m_keten;

		std::vector<Transaction> m_pendingTransactions;
	
		std::jthread m_messageProcessingThread;
	};

}