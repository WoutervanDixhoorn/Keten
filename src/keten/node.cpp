#include "node.h"
#include "crypto.h"

#include <print>
#include <sstream>
#include <iostream>
#include <cstdint>

namespace Keten {

	Node::Node(const std::string nodePort, const std::string seedIp /* = ""*/, const std::string seedPort /*= ""*/)
		: m_nodePort(nodePort), m_seedIpAddr(seedIp), m_seedPort(seedPort)
	{
		generateKeyPair(m_id.publicKey, m_id.privateKey);

		initializeP2P();
	}

	Node::~Node() {
		if (m_serverThread.joinable()) m_serverThread.join();
		if (m_clientThread.joinable()) m_clientThread.join();

		msock_deinit();
	}

	void Node::Start() {
		std::println("Start Keten Node...");
	
		m_serverThread = std::jthread(&Node::startListenServer, this);

		if (!m_seedIpAddr.empty() && !m_seedPort.empty()) {
			m_clientThread = std::jthread(&Node::startListenClient, this);
		}

		handleUserInput();
	}

	void Node::handleUserInput() {
		std::string input;
		while (true) {
			std::getline(std::cin, input);

			if (input == "exit") {
				msock_client_close(&m_client);
				msock_server_close(&m_server);
				break;
			}
			else if (input == "info") {
				std::println("My Public Key: {}", m_id.publicKey);
			} else if (input.starts_with("send")) {
				std::istringstream ss(input);

				std::string command;
				std::string receiverKey;
				double amount;

				if (ss >> command >> receiverKey >> amount) {
					std::println("Drawfting transaction of {} coins to {}...", amount, receiverKey.substr(0, 6));
				}
				else {
					std::println("Invalid format! Use: send <key> <amount>");
					continue;
				}

				Keten::Transaction transaction;
				transaction.amount = amount;
				transaction.sender = m_id.publicKey;
				transaction.receiver = receiverKey;

				std::string transactionData = transaction.getRawData();
				std::string transactionHash;
				picosha2::hash256(transactionData, transactionHash);
				std::string signature = signMessage(transactionHash, m_id.privateKey);
				transaction.signature = signature;

				m_pendingTransactions.push_back(transaction);

				std::string txJson = transaction.toJson();
				msock_message txMsg = {
					.buffer = txJson.data(),
					.size = txJson.size(),
					.len = txJson.length()
				};

				if (msock_client_is_connected(&m_client)) {
					msock_client_send(&m_client, &txMsg);
				}
			}
		}
	}

	void Node::initializeP2P() {
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
	}

	void Node::startListenClient() {
		if (!msock_client_connect(&m_client, m_seedIpAddr.c_str(), m_seedPort.c_str())) {
			std::println("[CLIENT] Failed connecting to: {}:{}", m_seedIpAddr, m_seedPort);
		}

		//TODO: Specify standard buffer size
		char receive_buffer[1024];
		msock_message msg = {
			.buffer = receive_buffer,
			.size = 1024
		};
		while (msock_client_is_connected(&m_client)) {

			if (msock_client_receive(&m_client, &msg)) {
				onNodeReceive(msg);
			}
			else {
				break;
			}
				
			//TODO: Maybe sleep
		}

		msock_client_close(&m_client);
	}

	void Node::onNodeReceive(msock_message& msg) {
		processIncomingMessage(msg);
		//TODO: Validate build the block etc..
	}

	void Node::startListenServer() {
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

	bool Node::onClientNodeConnect(msock_client* client) {
		std::println("[SERVER] Client just connected");
		return true;
	}

	bool Node::onClientNodeDisconnect(msock_client* client) {
		std::println("[SERVER] Client just disconnected");
		return true;
	}

	bool Node::onHandleClient(msock_server* server, msock_client* client) {
		char buffer[2048];
		msock_message txMsg = {
			.buffer = buffer,
			.size = sizeof(buffer)
		};

		// 1. Use ssize_t!
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

		if (static_cast<Node*>(server->userdata)->processIncomingMessage(txMsg)) {
			msock_server_broadcast(server, &txMsg, client);
		}
		
		return true;
	}

	bool Node::processIncomingMessage(msock_message& message)
	{
		json msgJson = json::parse(message.buffer);
		Transaction incomingTransaction(msgJson);
	
		std::println("Message from {}\nAmount: {}", incomingTransaction.sender.substr(0, 6), incomingTransaction.amount);

		return true;
	}

}