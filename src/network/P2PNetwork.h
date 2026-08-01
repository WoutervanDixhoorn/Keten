#pragma once

#include "messageQueue.h"
#include "messageTypes.h"

extern "C" {
	#include "msock.h"
}

#include <thread>
#include <mutex>
#include <unordered_map>

namespace Keten {

	class P2PNetwork {
	public:
		P2PNetwork(const std::string& nodePort, const std::string& seedIp = "", const std::string& seedPort = "");
		~P2PNetwork();

		void Start();
		void Stop();

		void PushMessage(const NetworkMessage& message);
		bool PollMessage(NodeMessage& outMessage);

	private:
		bool initializeSockets();

		void processNodeMessages();

		void startListenClient();
		void onNodeReceive(const std::string& msgPayload);

		void startListenServer();
		static bool onClientNodeConnect(msock_client* client);
		static bool onClientNodeDisconnect(msock_client* client);
		static bool onHandleClient(msock_server* server, msock_client* client);

	private:
		MessageQueue<NetworkMessage> m_inboundQueue;
		MessageQueue<NodeMessage> m_outboundQueue;

		std::string m_seedIpAddr;
		std::string m_seedPort;
		std::string m_nodePort;

		msock_client m_client;
		msock_server m_server;
		std::jthread m_serverThread;
		std::jthread m_clientThread;
		std::jthread m_nodeMessageThread;


		const size_t MAX_MESSAGE_SIZE = 8192;
		std::unordered_map<msock_client*, std::string> m_clientBuffers;

		std::mutex m_mapMutex;
		std::unordered_map<std::string, msock_client*> m_nodeToClientMap;
	};

}