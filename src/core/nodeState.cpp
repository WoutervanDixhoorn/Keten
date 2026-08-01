#include "nodeState.h"

#include "node.h"
#include "network/messageFactory.h"

#include <print>


namespace Keten {

	void ActiveState::operator()(const NodeBeginSyncEvent& event, Node& node)
	{
		//DROP EVENT DURING ACTIVE
		(void)event;
		(void)node;
	}

	void ActiveState::operator()(const NodeSyncRequestEvent& event, Node& node)
	{
		uint32_t targetChainHeight = node.GetKeten().Size();
		
		const std::vector<Block>& chain = node.GetKeten().GetChain();
		for (size_t i = event.currentHeight; i < chain.size(); i += 50) { // TODO: Define batch_size somewhere
			size_t endIndex = (std::min)(i + 50, chain.size());
			std::vector<Block> blockBatch(chain.begin() + i, chain.begin() + endIndex);
			nlohmann::json payload = { {"targetChainHeight", targetChainHeight}, {"syncBlocks", blockBatch}, {"targetNodeId", event.requesterId} };

			NetworkMessage syncResMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::SYNC_RESPONSE, payload, NetworkMessageType::DIRECT);
			node.PushNetworkMessage(syncResMessage);
		}
	}

	void ActiveState::operator()(const NodeSyncResponseEvent& event, Node& node)
	{
		//DROP EVENT DURING ACTIVE
		(void)event;
		(void)node;
	}

	void ActiveState::operator()(const NodeTransactionEvent& event, Node& node)
	{
		long senderBalance = node.GetBalance(event.tx.sender);
		if (!node.GetTransactionManager().ValidateTransaction(event.tx, senderBalance))
		{
			//std::println("Transaction is not valid!");
			return;
		}

		node.GetTransactionManager().AddTransaction(event.tx);

		//std::println("Transaction is valid!");

		NetworkMessage netMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::TRANSACTION, event.tx, NetworkMessageType::BROADCAST);
		node.PushNetworkMessage(netMessage);
	}

	void ActiveState::operator()(const NodeNewBlockEvent& event, Node& node)
	{
		if (!node.GetKeten().AddBlock(event.newBlock))
		{
			// TODO: Find reason why block is not accepted and deal with it (Ignore, Re-sync, ...)

			return;
		}

		node.GetTransactionManager().ApplyBlock(event.newBlock);
		node.GetLedger().ApplyBlock(event.newBlock);

		NetworkMessage netMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::BLOCK, event.newBlock, NetworkMessageType::BROADCAST);
		node.PushNetworkMessage(netMessage);
	}

	// ----------------------------------------------------------------------

	void SyncingState::operator()(const NodeBeginSyncEvent& event, Node& node)
	{
		uint32_t myHeight = node.GetChainHeight();

		nlohmann::json payload = {
			{"currentHeight", myHeight},
			{"nodeId", node.GetPublicKey()}
		};

		NetworkMessage netMessage = MessageFactory::CreateNetworkMessage(NodeMessageType::SYNC_REQUEST, payload, NetworkMessageType::DIRECT);
		node.PushNetworkMessage(netMessage);
	}

	void SyncingState::operator()(const NodeSyncRequestEvent& event, Node& node)
	{
		//DROP EVENT DURING SYNCING
		(void)event;
		(void)node;
	}

	void SyncingState::operator()(const NodeSyncResponseEvent& event, Node& node)
	{
		std::println("Node is syncing! Amount of first block batch: {}", event.syncBlocks.size());

		// TODO: Maybe reintroduce the ProcessSync but add the ledger to the blockManager, or completely move away from the managers for this kind of stuff and keep it as is
		//node.GetBlockManager().ProcessSync(event.syncBlocks);
		for (auto& block : event.syncBlocks) {
			node.GetKeten().AddBlock(block);
			node.GetLedger().ApplyBlock(block);
		}
		
		if (node.GetChainHeight() >= event.targetChainHeight)
		{
			node.SetState(ActiveState());
		}
	}

	void SyncingState::operator()(const NodeTransactionEvent& event, Node& node)
	{
		//DROP EVENT DURING SYNC
		(void)event;
		(void)node;
	}

	void SyncingState::operator()(const NodeNewBlockEvent& event, Node& node)
	{
		//DROP EVENT DURING SYNC
		(void)event;
		(void)node;
	}

}