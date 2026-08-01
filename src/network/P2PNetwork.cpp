#include "P2PNetwork.h"

#include "messageFactory.h"

#include <print>
#include <cstdint>
#include <utility>

#include "json.hpp"
using json = nlohmann::json;


namespace Keten {

	P2PNetwork::P2PNetwork(const std::string& nodePort, const std::string& seedIp, const std::string& seedPort) 
		: m_nodePort(nodePort), m_seedIpAddr(seedIp), m_seedPort(seedPort)
	{
		(bool) initializeSockets();
	}

	P2PNetwork::~P2PNetwork()
	{
		if (m_serverThread.joinable()) m_serverThread.join();
		if (m_clientThread.joinable()) m_clientThread.join();
		if (m_nodeMessageThread.joinable()) m_nodeMessageThread.join();

		msock_deinit();
	}

	void P2PNetwork::Start() 
	{
		m_nodeMessageThread = std::jthread(&P2PNetwork::processNodeMessages, this);
		m_serverThread = std::jthread(&P2PNetwork::startListenServer, this);

		if (!m_seedIpAddr.empty() && !m_seedPort.empty()) 
		{
			m_clientThread = std::jthread(&P2PNetwork::startListenClient, this);
		}
	}

	void P2PNetwork::Stop() 
	{
		msock_client_close(&m_client);
		msock_server_close(&m_server);
	}

	bool P2PNetwork::PollMessage(NodeMessage& outMessage) 
	{
		return m_outboundQueue.TryPop(outMessage);
	}

	void P2PNetwork::PushMessage(const NetworkMessage& message) 
	{
		m_inboundQueue.Push(message);
	}

	bool P2PNetwork::initializeSockets() 
	{
		msock_init();
		msock_server_create(&m_server);

		if (!m_seedIpAddr.empty() && !m_seedPort.empty())
		{
			msock_client_create(&m_client);
		}
		else
		{
			std::println("[NODE] No seed ip and port were provided.\nPlease connect using 'connect <port> <ip>");
		}

		//NOTE: C -> C++ gap
		msock_server_set_userdata(&m_server, this);
		msock_client_set_userdata(&m_client, this);

		return true;
	}

	void P2PNetwork::processNodeMessages() 
	{

		while (true) 
		{

			NetworkMessage message;
			if (!m_inboundQueue.TryPop(message)) 
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			msock_message outMsg;
			std::string safeMsgPayload = message.payload.append("\n");
			outMsg.buffer = safeMsgPayload.data();
			outMsg.size = safeMsgPayload.size();
			outMsg.len = safeMsgPayload.length();

			switch (message.messageType)
			{
			case NetworkMessageType::BROADCAST:
				msock_server_broadcast(&m_server, &outMsg, m_client.socket_state == MSOCK_STATE_CONNECTED ? &m_client : nullptr);
				if (msock_client_is_connected(&m_client)) {
					msock_client_send(&m_client, &outMsg);
				}
				break;
			case NetworkMessageType::DIRECT:
				try {
					auto j = nlohmann::json::parse(message.payload);
					std::string targetId = j["data"]["targetNodeId"].get<std::string>();

					std::lock_guard<std::mutex> lock(m_mapMutex);
					if (m_nodeToClientMap.contains(targetId)) {
						msock_client* targetClient = m_nodeToClientMap[targetId];
						msock_client_send(targetClient, &outMsg);
						break;
					}
				}
				catch (...) { /*Ignore*/ }

				if (msock_client_is_connected(&m_client)) {
					msock_client_send(&m_client, &outMsg);
				}
				break;
			}

		}
	}

	void P2PNetwork::startListenClient()
	{
		if (!msock_client_connect(&m_client, m_seedIpAddr.c_str(), m_seedPort.c_str())) 
		{
			std::println("[CLIENT] Failed connecting to: {}:{}", m_seedIpAddr, m_seedPort);
		}

		//TODO: Specify standard buffer size
		char receive_buffer[2048];
		msock_message msg = {
			.buffer = receive_buffer,
			.size = sizeof(receive_buffer)
		};

		std::string m_serverBuffer;

		while (msock_client_is_connected(&m_client)) 
		{

			ssize_t bytes = msock_client_receive(&m_client, &msg);

			if (bytes < 0) 
			{
				printf("Client receive failed!\n");
				return;
			}

			if (bytes == 0) 
			{
				if (m_client.socket_state == MSOCK_STATE_DISCONNECTED) 
				{
					return;
				}
			}

			m_serverBuffer.append(msg.buffer, msg.len);

			ssize_t pos;
			while((pos = m_serverBuffer.find('\n')) != std::string::npos) 
			{
				std::string frame = m_serverBuffer.substr(0, pos);
				m_serverBuffer.erase(0, pos + 1);

				if (!frame.empty())
				{
					onNodeReceive(frame);
				}
			}
		}

		msock_client_close(&m_client);
	}

	void P2PNetwork::onNodeReceive(const std::string& msgFrame)
	{
		std::optional<NodeMessage> parsedMsg = MessageFactory::ParseNetworkFrame(msgFrame);
		if(parsedMsg.has_value())
		{
			m_outboundQueue.Push(parsedMsg.value());
		}
		else
		{
			std::println("[CLIENT] Invalid JSON envelope received.");
		}
	}

	void P2PNetwork::startListenServer()
	{
		//NOTE: For now we use localhost, this will be 0.0.0.0 in the future. need to imporve useability
		if (!msock_server_listen(&m_server, "127.0.0.1", m_nodePort.c_str())) 
		{
			std::println("[SERVER] Failed listening on: {}:{}", "127.0.0.1", m_nodePort);
		}

		msock_server_set_connect_cb(&m_server, onClientNodeConnect);
		msock_server_set_disconnect_cb(&m_server, onClientNodeDisconnect);
		msock_server_set_client_cb(&m_server, onHandleClient);

		while (msock_server_is_listening(&m_server))
		{
			msock_server_run(&m_server);
		}

		msock_server_close(&m_server);
	}

	bool P2PNetwork::onClientNodeConnect(msock_client* client) 
	{
		std::println("[SERVER] Client just connected");
		return true;
	}

	bool P2PNetwork::onClientNodeDisconnect(msock_client* client) 
	{
		std::println("[SERVER] Client just disconnected");
		auto* net = static_cast<P2PNetwork*>(client->userdata);
		net->m_clientBuffers.erase(client);

		std::lock_guard<std::mutex> lock(net->m_mapMutex);
		std::erase_if(net->m_nodeToClientMap, [client](const auto& item) {
			return item.second == client;
		});

		return true;
	}

	bool P2PNetwork::onHandleClient(msock_server* server, msock_client* client)
	{
		char buffer[2048];
		msock_message txMsg = {
			.buffer = buffer,
			.size = sizeof(buffer)
		};
		
		ssize_t bytes = msock_client_receive(client, &txMsg);

		if (bytes < 0) 
		{
			printf("Client receive failed!\n");
			return false;
		}

		if (bytes == 0) 
		{
			if (client->socket_state == MSOCK_STATE_DISCONNECTED)
			{
				return false;
			}
			return true;
		}

		P2PNetwork* net = static_cast<P2PNetwork*>(server->userdata);
		net->m_clientBuffers[client].append(txMsg.buffer, txMsg.len);

		if (net->m_clientBuffers[client].size() > net->MAX_MESSAGE_SIZE) 
		{
			std::println("[SECURITY] Client exceeded max buffer size! Dropping connection.");
			net->m_clientBuffers.erase(client);
			return false;
		}

		ssize_t pos;

		while ((pos = net->m_clientBuffers[client].find('\n')) != std::string::npos)
		{
			std::string frame = net->m_clientBuffers[client].substr(0, pos);
			net->m_clientBuffers[client].erase(0, pos + 1);

			if (!frame.empty()) 
			{
				std::optional<NodeMessage> parsedMsg = MessageFactory::ParseNetworkFrame(frame);
				if (parsedMsg.has_value()) 
				{
					try {
						auto j = nlohmann::json::parse(parsedMsg.value().payload);
						if (j.contains("nodeId")) {
							std::string nodeId = j["nodeId"];
							std::lock_guard<std::mutex> lock(net->m_mapMutex);
							net->m_nodeToClientMap[nodeId] = client;
						}
					}
					catch (...) { /*Ignore*/ }

					net->m_outboundQueue.Push(parsedMsg.value());
				}
				else
				{
					std::println("[SECURITY] Invalid JSON envelope received. Dropping frame.");
				}
			}
		}
		
		return true;
	}
}