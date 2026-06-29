#pragma once
#include "blockchain.h"
#include "types.h"

#include <string>
#include <vector>
#include <thread>

extern "C" {
	#include "msock.h"
}

namespace Keten {

	typedef struct {
		std::string publicKey;
		std::string privateKey;
	} NodeIdentity;

	class Node {
	public:
		Node(const std::string nodePort, const std::string seedIp = "", const std::string seedPort = "");
		~Node();

		void Start();

	private:
		void initializeP2P();

		void startListenClient();
		void onNodeReceive(msock_message& msg);

		void startListenServer();
		static bool onClientNodeConnect(msock_client* client);
		static bool onClientNodeDisconnect(msock_client* client);
		static bool onHandleClient(msock_server* server, msock_client* client);

		void handleUserInput();

		bool processIncomingMessage(msock_message& message);

	private:
		NodeIdentity m_id;
		Blockchain m_keten;
		
		std::vector<Transaction> m_pendingTransactions;

		std::string m_seedIpAddr;
		std::string m_seedPort;
		std::string m_nodePort;

		msock_client m_client;
		msock_server m_server;
		std::jthread m_serverThread;
		std::jthread m_clientThread;
	};

}