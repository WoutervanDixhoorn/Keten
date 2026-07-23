#include "keten.h"

#include <thread>

#define TRANSACTION_COUNT 1000

int main() {
	Keten::Node seedNode("4455");
	seedNode.Start(false);

	Keten::Node nodeOne("4456", "127.0.0.1", "4455");
	nodeOne.Start(false);
	Keten::Node nodeTwo("4457", "127.0.0.1", "4455");
	nodeTwo.Start(false);


	for (int i = 0; i < TRANSACTION_COUNT; i++) {
		nodeOne.SendTransaction(10, nodeTwo.GetPublicKey());
		nodeTwo.SendTransaction(5312, nodeOne.GetPublicKey());
	}

	for (int i = 0; i < TRANSACTION_COUNT; i++) {
		nodeOne.SendTransaction(1421, seedNode.GetPublicKey());
	}

	return 0;
}