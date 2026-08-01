#pragma once

#include "models/block.h"

#include <vector>

namespace Keten {
	class Node;

	struct NodeBeginSyncEvent {};

	struct NodeSyncRequestEvent {
		uint32_t currentHeight;
		std::string requesterId;
	};

	struct NodeSyncResponseEvent {
		uint32_t targetChainHeight;
		std::vector<Block> syncBlocks;
	};

	struct NodeTransactionEvent {
		Transaction tx;
	};

	struct NodeNewBlockEvent {
		Block newBlock;
	};

	struct ActiveState {
		void operator()(const NodeBeginSyncEvent& event, Node& node);
		void operator()(const NodeSyncRequestEvent& event, Node& node);
		void operator()(const NodeSyncResponseEvent& event, Node& node);

		void operator()(const NodeTransactionEvent& event, Node& node);
		void operator()(const NodeNewBlockEvent& event, Node& node);
	};

	struct SyncingState {
		void operator()(const NodeBeginSyncEvent& event, Node& node);
		void operator()(const NodeSyncRequestEvent& event, Node& node);
		void operator()(const NodeSyncResponseEvent& event, Node& node);

		void operator()(const NodeTransactionEvent& event, Node& node);
		void operator()(const NodeNewBlockEvent& event, Node& node);
	};

}