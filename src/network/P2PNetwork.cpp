#include "P2PNetwork.h"

#include <print>
#include <cstdint>
#include <utility>

#include "json.hpp"
using json = nlohmann::json;


namespace Keten {

	P2PNetwork::P2PNetwork(const std::string nodePort, const std::string seedIp /* = ""*/, const std::string seedPort /*= ""*/) 
		: m_nodePort(nodePort), m_seedIpAddr(seedIp), m_seedPort(seedPort)
	{
		(bool) initializeSockets();
	}

	P2PNetwork::~P2PNetwork() {
		if (m_serverThread.joinable()) m_serverThread.join();
		if (m_clientThread.joinable()) m_clientThread.join();
		if (m_nodeMessageThread.joinable()) m_nodeMessageThread.join();

		msock_deinit();
	}

	void P2PNetwork::Start() {
		m_serverThread = std::jthread(&P2PNetwork::startListenServer, this);

		if (!m_seedIpAddr.empty() && !m_seedPort.empty()) {
			m_clientThread = std::jthread(&P2PNetwork::startListenClient, this);
		}

		m_nodeMessageThread = std::jthread(&P2PNetwork::processNodeMessages, this);
	}

	void P2PNetwork::Stop() {
		msock_client_close(&m_client);
		msock_server_close(&m_server);
	}

	bool P2PNetwork::PollMessage(NodeMessage& outMessage) {
		return m_outboundQueue.TryPop(outMessage);
	}

	void P2PNetwork::PushMessage(NetworkMessage& message) {
		m_inboundQueue.Push(message);
	}

	bool P2PNetwork::initializeSockets() {
		msock_init();
		msock_server_create(&m_server);

		if (!m_seedIpAddr.empty() && !m_seedPort.empty()) {
			msock_client_create(&m_client);
		}
		else {
			std::println("[NODE] No seed ip and port were provided.\nPlease connect using 'connect <port> <ip>");
		}

		//NOTE: C -> C++ gap
		msock_server_set_userdata(&m_server, this);
		msock_client_set_userdata(&m_client, this);

		return true;
	}

	void P2PNetwork::processNodeMessages() {

		while (true) {

			NetworkMessage message;
			if (!m_inboundQueue.TryPop(message)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			msock_message outMsg;
			outMsg.buffer = message.payload.data();
			outMsg.size = message.payload.size();
			outMsg.len = message.payload.length();

			switch (message.messageType) {
			case NetworkMessageType::BOARDCAST:
				msock_server_broadcast(&m_server, &outMsg, m_client.socket_state == MSOCK_STATE_CONNECTED ? &m_client : nullptr);
				break;
			case NetworkMessageType::DIRECT:
				if (msock_client_is_connected(&m_client)) {
					msock_client_send(&m_client, &outMsg);
				}
				break;
			}

		}
	}

	void P2PNetwork::startListenClient() {
		if (!msock_client_connect(&m_client, m_seedIpAddr.c_str(), m_seedPort.c_str())) {
			std::println("[CLIENT] Failed connecting to: {}:{}", m_seedIpAddr, m_seedPort);
		}

		//TODO: Specify standard buffer size
		char receive_buffer[2048];
		msock_message msg = {
			.buffer = receive_buffer,
			.size = sizeof(receive_buffer)
		};
		while (msock_client_is_connected(&m_client)) {

			ssize_t bytes = msock_client_receive(&m_client, &msg);

			if (bytes < 0) {
				printf("Client receive failed!\n");
				return;
			}

			if (bytes == 0) {
				if (m_client.socket_state == MSOCK_STATE_DISCONNECTED) {
					return;
				}
			}

			onNodeReceive(msg);
		}

		msock_client_close(&m_client);
	}

	void P2PNetwork::onNodeReceive(msock_message& msg) {
		NodeMessage outMsg = {
			.payload = std::move(msg.buffer),
			.messageType = NodeMessageType::TRANSACTION
		};

		m_outboundQueue.Push(outMsg);
	}

	void P2PNetwork::startListenServer() {
		//NOTE: For now we use localhost, this will be 0.0.0.0 in the future. need to imporve useability
		if (!msock_server_listen(&m_server, "127.0.0.1", m_nodePort.c_str())) {
			std::println("[SERVER] Failed listening on: {}:{}", "127.0.0.1", m_nodePort);
		}

		msock_server_set_connect_cb(&m_server, onClientNodeConnect);
		msock_server_set_disconnect_cb(&m_server, onClientNodeDisconnect);
		msock_server_set_client_cb(&m_server, onHandleClient);

		while (msock_server_is_listening(&m_server)) {
			msock_server_run(&m_server);
		}

		msock_server_close(&m_server);
	}

	bool P2PNetwork::onClientNodeConnect(msock_client* client) {
		std::println("[SERVER] Client just connected");
		return true;
	}

	bool P2PNetwork::onClientNodeDisconnect(msock_client* client) {
		std::println("[SERVER] Client just disconnected");
		return true;
	}

	bool P2PNetwork::onHandleClient(msock_server* server, msock_client* client) {
		char buffer[2048];
		msock_message txMsg = {
			.buffer = buffer,
			.size = sizeof(buffer)
		};
		
		ssize_t bytes = msock_client_receive(client, &txMsg);

		if (bytes < 0) {
			printf("Client receive failed!\n");
			return false;
		}

		if (bytes == 0) {
			if (client->socket_state == MSOCK_STATE_DISCONNECTED) {
				return false;
			}
			return true;
		}

		std::println("[SERVER] Received message");

		NodeMessage outMsg = {
			.payload = std::string(txMsg.buffer, bytes),
			.messageType = NodeMessageType::TRANSACTION
		};

		static_cast<P2PNetwork*>(server->userdata)->m_outboundQueue.Push(outMsg);

		return true;
	}
}