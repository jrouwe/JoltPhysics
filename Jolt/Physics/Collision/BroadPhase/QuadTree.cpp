// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>
#include <Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <Physics/Collision/RayCast.h>
#include <Physics/Collision/AABoxCast.h>
#include <Physics/Collision/CastResult.h>
#include <Physics/Body/BodyPair.h>
#include <Physics/PhysicsLock.h>
#include <Geometry/AABox4.h>
#include <Geometry/RayAABox.h>
#include <Geometry/OrientedBox.h>

namespace JPH {

////////////////////////////////////////////////////////////////////////////////////////////////////////
// QuadTree::Node
////////////////////////////////////////////////////////////////////////////////////////////////////////

QuadTree::Node::Node(bool inLocked)
{
	// First reset bounds
	Vec4 val = Vec4::sReplicate(cLargeFloat);
	val.StoreFloat4((Float4 *)&mBoundsMinX);
	val.StoreFloat4((Float4 *)&mBoundsMinY);
	val.StoreFloat4((Float4 *)&mBoundsMinZ);
	val = Vec4::sReplicate(-cLargeFloat);
	val.StoreFloat4((Float4 *)&mBoundsMaxX);
	val.StoreFloat4((Float4 *)&mBoundsMaxY);
	val.StoreFloat4((Float4 *)&mBoundsMaxZ);

	// Reset child node ids
	mChildNodeID[0] = NodeID::sInvalid();
	mChildNodeID[1] = NodeID::sInvalid();
	mChildNodeID[2] = NodeID::sInvalid();
	mChildNodeID[3] = NodeID::sInvalid();

	// Reset parent node index
	mParentNodeIndex = cInvalidNodeIndex;

	// Store lock status
	mIsLocked = inLocked;
}

void QuadTree::Node::GetChildBounds(int inChildIndex, AABox &outBounds) const
{
	// Read bounding box in order min -> max
	outBounds.mMin = Vec3(mBoundsMinX[inChildIndex], mBoundsMinY[inChildIndex], mBoundsMinZ[inChildIndex]);
	outBounds.mMax = Vec3(mBoundsMaxX[inChildIndex], mBoundsMaxY[inChildIndex], mBoundsMaxZ[inChildIndex]);
}

void QuadTree::Node::SetChildBounds(int inChildIndex, const AABox &inBounds)
{
	// Set max first (this keeps the bounding box invalid for reading threads)
	mBoundsMaxZ[inChildIndex] = inBounds.mMax.GetZ();
	mBoundsMaxY[inChildIndex] = inBounds.mMax.GetY();
	mBoundsMaxX[inChildIndex] = inBounds.mMax.GetX();

	// Then set min (and make box valid)
	mBoundsMinZ[inChildIndex] = inBounds.mMin.GetZ();
	mBoundsMinY[inChildIndex] = inBounds.mMin.GetY();
	mBoundsMinX[inChildIndex] = inBounds.mMin.GetX(); // Min X becomes valid last
}

void QuadTree::Node::InvalidateChildBounds(int inChildIndex)
{
	// First we make the box invalid by setting the min to cLargeFloat
	mBoundsMinX[inChildIndex] = cLargeFloat; // Min X becomes invalid first
	mBoundsMinY[inChildIndex] = cLargeFloat;
	mBoundsMinZ[inChildIndex] = cLargeFloat;

	// Then we reset the max values too
	mBoundsMaxX[inChildIndex] = -cLargeFloat;
	mBoundsMaxY[inChildIndex] = -cLargeFloat;
	mBoundsMaxZ[inChildIndex] = -cLargeFloat;
}

void QuadTree::Node::GetNodeBounds(AABox &outBounds) const
{
	// Get first child bounds
	GetChildBounds(0, outBounds);

	// Encapsulate other child bounds
	for (int child_idx = 1; child_idx < 4; ++child_idx)
	{
		AABox tmp;
		GetChildBounds(child_idx, tmp);
		outBounds.Encapsulate(tmp);
	}
}

bool QuadTree::Node::EncapsulateChildBounds(int inChildIndex, const AABox &inBounds)
{
	bool changed = AtomicMin(mBoundsMinX[inChildIndex], inBounds.mMin.GetX());
	changed |= AtomicMin(mBoundsMinY[inChildIndex], inBounds.mMin.GetY());
	changed |= AtomicMin(mBoundsMinZ[inChildIndex], inBounds.mMin.GetZ());
	changed |= AtomicMax(mBoundsMaxX[inChildIndex], inBounds.mMax.GetX());
	changed |= AtomicMax(mBoundsMaxY[inChildIndex], inBounds.mMax.GetY());
	changed |= AtomicMax(mBoundsMaxZ[inChildIndex], inBounds.mMax.GetZ());
	return changed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// QuadTree
////////////////////////////////////////////////////////////////////////////////////////////////////////

const float QuadTree::cLargeFloat = 1.0e30f;
const AABox QuadTree::cInvalidBounds(Vec3::sReplicate(cLargeFloat), Vec3::sReplicate(-cLargeFloat));

void QuadTree::GetBodyLocation(const TrackingVector &inTracking, BodyID inBodyID, uint32 &outNodeIdx, uint32 &outChildIdx) const
{
	uint32 body_location = inTracking[inBodyID.GetIndex()].mBodyLocation;
	JPH_ASSERT(body_location != Tracking::cInvalidBodyLocation);
	outNodeIdx = body_location & 0x3fffffff;
	outChildIdx = body_location >> 30;
	JPH_ASSERT(mAllocator->Get(outNodeIdx).mChildNodeID[outChildIdx] == inBodyID, "Make sure that the body is in the node where it should be");
}

void QuadTree::SetBodyLocation(TrackingVector &ioTracking, BodyID inBodyID, uint32 inNodeIdx, uint32 inChildIdx) const
{ 
	JPH_ASSERT(inNodeIdx <= 0x3fffffff);
	JPH_ASSERT(inChildIdx < 4); 
	JPH_ASSERT(mAllocator->Get(inNodeIdx).mChildNodeID[inChildIdx] == inBodyID, "Make sure that the body is in the node where it should be");
	ioTracking[inBodyID.GetIndex()].mBodyLocation = inNodeIdx + (inChildIdx << 30);

#ifdef JPH_ENABLE_ASSERTS
	uint32 v1, v2;
	GetBodyLocation(ioTracking, inBodyID, v1, v2);
	JPH_ASSERT(v1 == inNodeIdx);
	JPH_ASSERT(v2 == inChildIdx);
#endif
}

void QuadTree::InvalidateBodyLocation(TrackingVector &ioTracking, BodyID inBodyID)
{
	ioTracking[inBodyID.GetIndex()].mBodyLocation = Tracking::cInvalidBodyLocation;
}

QuadTree::~QuadTree()
{
	// Get rid of any nodes that are still to be freed
	DiscardOldTree();

	// Get the current root node
	RootNode &root_node = GetCurrentRoot();

	// Collect all bodies
	Allocator::Batch free_batch;
	NodeID node_stack[cStackSize];
	node_stack[0] = root_node.GetNodeID();
	JPH_ASSERT(node_stack[0].IsValid());
	if (node_stack[0].IsNode())
	{
		int top = 0;
		do
		{
			// Process node
			NodeID node_id = node_stack[top];
			JPH_ASSERT(!node_id.IsBody());
			uint32 node_idx = node_id.GetNodeIndex();
			const Node &node = mAllocator->Get(node_idx);

			// Recurse and get all child nodes
			for (int i = 0; i < 4; ++i)
			{
				NodeID child_node_id = node.mChildNodeID[i];
				if (child_node_id.IsValid() && child_node_id.IsNode())
				{
					JPH_ASSERT(top < cStackSize);
					node_stack[top] = child_node_id;
					top++;
				}
			}

			// Mark node to be freed
			mAllocator->AddObjectToBatch(free_batch, node_idx);
			--top;
		} 
		while (top >= 0);
	}

	// Now free all nodes
	mAllocator->DestructObjectBatch(free_batch);
}

uint32 QuadTree::AllocateNode(bool inLocked)
{
	uint32 index = mAllocator->ConstructObject(inLocked);
	if (index == Allocator::cInvalidObjectIndex)
	{
		Trace("QuadTree: Out of nodes!");
		JPH_CRASH;
	}
	return index;
}

void QuadTree::Init(Allocator &inAllocator)
{
	// Store allocator
	mAllocator = &inAllocator;
	
	// Allocate root node
	mRootNode[mRootNodeIndex].mIndex = AllocateNode(false);
}

void QuadTree::DiscardOldTree()
{
	// Check if there is an old tree
	RootNode &old_root_node = mRootNode[mRootNodeIndex ^ 1];
	if (old_root_node.mIndex != cInvalidNodeIndex)
	{
		// Clear the root
		old_root_node.mIndex = cInvalidNodeIndex;

		// Now free all old nodes
		mAllocator->DestructObjectBatch(mFreeNodeBatch);

		// Clear the batch
		mFreeNodeBatch = Allocator::Batch();
	}
}

void QuadTree::UpdatePrepare(const BodyVector &inBodies, TrackingVector &ioTracking, UpdateState &outUpdateState)
{
#ifdef JPH_ENABLE_ASSERTS
	// We only read positions
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);
#endif

	// Assert we have no nodes pending deletion, this means DiscardOldTree wasn't called yet
	JPH_ASSERT(mFreeNodeBatch.mNumObjects == 0);

	// Mark tree non-dirty
	mIsDirty = false;

	// Get the current root node
	RootNode &root_node = GetCurrentRoot();

	// Assert sane data
#ifdef _DEBUG
	ValidateTree(inBodies, ioTracking, root_node.mIndex, mNumBodies);
#endif

	// Create space for all body ID's
	outUpdateState.mAllNodeIDs = new NodeID [mNumBodies];
	NodeID *cur_node_id = outUpdateState.mAllNodeIDs;

	// Collect all bodies
	NodeID node_stack[cStackSize];
	node_stack[0] = root_node.GetNodeID();
	JPH_ASSERT(node_stack[0].IsValid());
	int top = 0;
	do
	{
		// Check if node is a body
		NodeID node_id = node_stack[top];
		if (node_id.IsBody())
		{
			// Validate that we're still in the right layer
		#ifdef JPH_ENABLE_ASSERTS
			uint32 body_index = node_id.GetBodyID().GetIndex();
			JPH_ASSERT(ioTracking[body_index].mObjectLayer == inBodies[body_index]->GetObjectLayer());
		#endif

			// Store body
			*cur_node_id = node_id;
			++cur_node_id;
		}
		else
		{
			// Process normal node
			uint32 node_idx = node_id.GetNodeIndex();
			const Node &node = mAllocator->Get(node_idx);

			if (node.mIsLocked)
			{
				// Node is locked, treat it as a whole
				*cur_node_id = node_id;
				++cur_node_id;
			}
			else
			{
				// Node is not locked, recurse and get all children
				for (int i = 0; i < 4; ++i)
				{
					NodeID child_node_id = node.mChildNodeID[i];
					if (child_node_id.IsValid())
					{
						JPH_ASSERT(top < cStackSize);
						node_stack[top] = child_node_id;
						top++;
					}
				}

				// Mark node to be freed
				mAllocator->AddObjectToBatch(mFreeNodeBatch, node_idx);
			}
		}
		--top;
	} 
	while (top >= 0);

	// Check that our book keeping matches
	uint32 num_node_ids = uint32(cur_node_id - outUpdateState.mAllNodeIDs);
	JPH_ASSERT(num_node_ids <= mNumBodies);

	// This will be the new root node id
	NodeID root_node_id;

	if (num_node_ids > 0)
	{
		// Build new tree
		AABox root_bounds;
		root_node_id = BuildTree(inBodies, ioTracking, outUpdateState.mAllNodeIDs, num_node_ids, cInvalidNodeIndex, false, root_bounds);

		if (root_node_id.IsBody())
		{
			// For a single body we need to allocate a new root node
			uint32 root_idx = AllocateNode(false);
			Node &root = mAllocator->Get(root_idx);
			root.SetChildBounds(0, root_bounds);
			root.mChildNodeID[0] = root_node_id;
			SetBodyLocation(ioTracking, root_node_id.GetBodyID(), root_idx, 0);
			root_node_id = NodeID::sFromNodeIndex(root_idx);
		}
		else
		{
			// We can't have a locked node as root, allocate a new root in this case
			Node &old_root_node = mAllocator->Get(root_node_id.GetNodeIndex());
			if (old_root_node.mIsLocked)
			{
				// Link old root under new root
				uint32 root_idx = AllocateNode(false);
				Node &root = mAllocator->Get(root_idx);
				root.SetChildBounds(0, root_bounds);
				root.mChildNodeID[0] = root_node_id;
				root_node_id = NodeID::sFromNodeIndex(root_idx);
				old_root_node.mParentNodeIndex = root_idx;
			}
		}
	}
	else
	{
		// Empty tree, create root node
		uint32 root_idx = AllocateNode(false);
		root_node_id = NodeID::sFromNodeIndex(root_idx);
	}

	outUpdateState.mRootNodeID = root_node_id;
}

void QuadTree::UpdateFinalize([[maybe_unused]] const BodyVector &inBodies, [[maybe_unused]] TrackingVector &ioTracking, UpdateState &inUpdateState)
{
	// Tree building is complete, now we switch the old with the new tree
	uint32 new_root_idx = mRootNodeIndex ^ 1;
	RootNode &new_root_node = mRootNode[new_root_idx];
	{
		// Note: We don't need to lock here as the old tree stays available so any queries 
		// that use it can continue using it until DiscardOldTree is called. This slot
		// should be empty and unused at this moment.
		JPH_ASSERT(new_root_node.mIndex == cInvalidNodeIndex);
		new_root_node.mIndex = inUpdateState.mRootNodeID.GetNodeIndex();
	}

	// All queries that start from now on will use this new tree
	mRootNodeIndex = new_root_idx;

	// Delete temporary data
	delete [] inUpdateState.mAllNodeIDs;

#ifdef _DEBUG
	ValidateTree(inBodies, ioTracking, new_root_node.mIndex, mNumBodies);
#endif
}

void QuadTree::sPartition(NodeID *ioNodeIDs, Vec3 *ioNodeCenters, int inNumber, int &outMidPoint)
{
	// Handle trivial case
	if (inNumber <= 4)
	{
		outMidPoint = inNumber / 2;
		return;
	}

	// Calculate bounding box of box centers
	Vec3 center_min = Vec3::sReplicate(cLargeFloat);
	Vec3 center_max = Vec3::sReplicate(-cLargeFloat);
	for (Vec3 *c = ioNodeCenters, *c_end = ioNodeCenters + inNumber; c < c_end; ++c)
	{
		Vec3 center = *c;
		center_min = Vec3::sMin(center_min, center);
		center_max = Vec3::sMax(center_max, center);
	}

	// Calculate split plane
	int dimension = (center_max - center_min).GetHighestComponentIndex();
	float split = 0.5f * (center_min + center_max)[dimension];

	// Divide bodies
	int start = 0, end = inNumber;
	while (start < end)
	{
		// Search for first element that is on the right hand side of the split plane
		while (start < end && ioNodeCenters[start][dimension] < split)
			++start;

		// Search for the first element that is on the left hand side of the split plane
		while (start < end && ioNodeCenters[end - 1][dimension] >= split)
			--end;

		if (start < end)
		{
			// Swap the two elements
			swap(ioNodeIDs[start], ioNodeIDs[end - 1]);
			swap(ioNodeCenters[start], ioNodeCenters[end - 1]);
			++start;
			--end;
		}
	}
	JPH_ASSERT(start == end);

	if (start > 0 && start < inNumber)
	{
		// Success!
		outMidPoint = start;
	}
	else
	{
		// Failed to divide bodies
		outMidPoint = inNumber / 2; 
	}
}

void QuadTree::sPartition4(NodeID *ioNodeIDs, Vec3 *ioNodeCenters, int inBegin, int inEnd, int *outSplit)
{
	NodeID *node_ids = ioNodeIDs + inBegin;
	Vec3 *node_centers = ioNodeCenters + inBegin;
	int number = inEnd - inBegin;

	// Partition entire range
	sPartition(node_ids, node_centers, number, outSplit[2]); 
	
	// Partition lower half
	sPartition(node_ids, node_centers, outSplit[2], outSplit[1]); 

	// Partition upper half
	sPartition(node_ids + outSplit[2], node_centers + outSplit[2], number - outSplit[2], outSplit[3]); 

	// Convert to proper range 
	outSplit[0] = inBegin;
	outSplit[1] += inBegin;
	outSplit[2] += inBegin;
	outSplit[3] += outSplit[2];
	outSplit[4] = inEnd;
}

AABox QuadTree::GetNodeOrBodyBounds(const BodyVector &inBodies, NodeID inNodeID) const
{
	if (inNodeID.IsNode())
	{
		// It is a node
		uint32 node_idx = inNodeID.GetNodeIndex();
		const Node &node = mAllocator->Get(node_idx);

		AABox bounds;
		node.GetNodeBounds(bounds);
		return bounds;
	}
	else
	{
		// It is a body
		return inBodies[inNodeID.GetBodyID().GetIndex()]->GetWorldSpaceBounds();
	}
}

QuadTree::NodeID QuadTree::BuildTree(const BodyVector &inBodies, TrackingVector &ioTracking, NodeID *ioNodeIDs, int inNumber, uint32 inParentNodeIndex, bool inLocked, AABox &outBounds)
{
	// Trivial case: No bodies in tree
	if (inNumber == 0)
	{
		outBounds = cInvalidBounds;
		return NodeID::sInvalid();
	}

	// Trivial case: When we have 1 body or node, return it
	if (inNumber == 1)
	{
		if (ioNodeIDs->IsNode())
		{
			// When returning an existing node as root, ensure that no parent has been set
			Node &node = mAllocator->Get(ioNodeIDs->GetNodeIndex());
			node.mParentNodeIndex = cInvalidNodeIndex;
		}
		outBounds = GetNodeOrBodyBounds(inBodies, *ioNodeIDs);
		return *ioNodeIDs;
	}

	// Calculate centers of all bodies that are to be inserted
	Vec3 *centers = new Vec3 [inNumber];
	JPH_ASSERT(IsAligned(centers, 16));
	Vec3 *c = centers;
	for (NodeID *n = ioNodeIDs, *n_end = ioNodeIDs + inNumber; n < n_end; ++n, ++c)
		*c = GetNodeOrBodyBounds(inBodies, *n).GetCenter();
	
	// The algorithm is a recursive tree build, but to avoid the call overhead we keep track of a stack here
	struct StackEntry
	{
		uint32			mNodeIdx;					// Node index of node that is generated
		int				mChildIdx;					// Index of child that we're currently processing
		int				mSplit[5];					// Indices where the node ID's have been split to form 4 partitions
		Vec3			mNodeBoundsMin;				// Bounding box of this node, accumulated while iterating over children
		Vec3			mNodeBoundsMax;
	};
	static_assert(sizeof(StackEntry) == 64);
	StackEntry stack[cStackSize / 4]; // We don't process 4 at a time in this loop but 1, so the stack can be 4x as small
	int top = 0;

	// Create root node
	stack[0].mNodeIdx = AllocateNode(inLocked);
	stack[0].mChildIdx = -1;
	stack[0].mNodeBoundsMin = Vec3::sReplicate(cLargeFloat);
	stack[0].mNodeBoundsMax = Vec3::sReplicate(-cLargeFloat);
	sPartition4(ioNodeIDs, centers, 0, inNumber, stack[0].mSplit);

	for (;;)
	{
		StackEntry &cur_stack = stack[top];

		// Next child
		cur_stack.mChildIdx++;

		// Check if all children processed
		if (cur_stack.mChildIdx >= 4)
		{
			// Terminate if there's nothing left to pop
			if (top <= 0)
				break;

			// Add our bounds to our parents bounds
			StackEntry &prev_stack = stack[top - 1];
			prev_stack.mNodeBoundsMin = Vec3::sMin(prev_stack.mNodeBoundsMin, cur_stack.mNodeBoundsMin);
			prev_stack.mNodeBoundsMax = Vec3::sMax(prev_stack.mNodeBoundsMax, cur_stack.mNodeBoundsMax);

			// Store parent node
			Node &node = mAllocator->Get(cur_stack.mNodeIdx);
			node.mParentNodeIndex = prev_stack.mNodeIdx;

			// Store this node's properties in the parent node
			Node &parent_node = mAllocator->Get(prev_stack.mNodeIdx);
			parent_node.mChildNodeID[prev_stack.mChildIdx] = NodeID::sFromNodeIndex(cur_stack.mNodeIdx);
			parent_node.SetChildBounds(prev_stack.mChildIdx, AABox(cur_stack.mNodeBoundsMin, cur_stack.mNodeBoundsMax));

			// Pop entry from stack
			--top;
		}
		else
		{
			// Get low and high index to bodies to process
			int low = cur_stack.mSplit[cur_stack.mChildIdx];
			int high = cur_stack.mSplit[cur_stack.mChildIdx + 1];
			int num_bodies = high - low;

			if (num_bodies == 1)
			{
				// Get body info
				NodeID child_node_id = ioNodeIDs[low];
				AABox bounds = GetNodeOrBodyBounds(inBodies, child_node_id);

				// Update node
				Node &node = mAllocator->Get(cur_stack.mNodeIdx);
				node.mChildNodeID[cur_stack.mChildIdx] = child_node_id;
				node.SetChildBounds(cur_stack.mChildIdx, bounds);

				if (child_node_id.IsNode())
				{
					// Update parent for this node
					Node &child_node = mAllocator->Get(child_node_id.GetNodeIndex());
					child_node.mParentNodeIndex = cur_stack.mNodeIdx;
				}
				else
				{
					// Set location in tracking
					SetBodyLocation(ioTracking, child_node_id.GetBodyID(), cur_stack.mNodeIdx, cur_stack.mChildIdx);
				}

				// Encapsulate bounding box in parent
				cur_stack.mNodeBoundsMin = Vec3::sMin(cur_stack.mNodeBoundsMin, bounds.mMin);
				cur_stack.mNodeBoundsMax = Vec3::sMax(cur_stack.mNodeBoundsMax, bounds.mMax);
			}
			else if (num_bodies > 1)
			{
				// Allocate new node
				StackEntry &new_stack = stack[++top];
				JPH_ASSERT(top < cStackSize / 4);
				new_stack.mNodeIdx = AllocateNode(inLocked);
				new_stack.mChildIdx = -1;
				new_stack.mNodeBoundsMin = Vec3::sReplicate(cLargeFloat);
				new_stack.mNodeBoundsMax = Vec3::sReplicate(-cLargeFloat);
				sPartition4(ioNodeIDs, centers, low, high, new_stack.mSplit);
			}
		}
	}

	// Delete temporary data
	delete [] centers;

	// Store bounding box of root
	outBounds.mMin = stack[0].mNodeBoundsMin;
	outBounds.mMax = stack[0].mNodeBoundsMax;

	// Return root
	return NodeID::sFromNodeIndex(stack[0].mNodeIdx);
}

void QuadTree::UnlockNodeAndParents(uint32 inNodeIndex)
{
	uint32 node_idx = inNodeIndex;

	do
	{
		// If node is not locked, parent won't be either
		Node &node = mAllocator->Get(node_idx);
		if (!node.mIsLocked)
			break;

		// Mark node as unlocked
		node.mIsLocked = false;

		// Get our parent
		node_idx = node.mParentNodeIndex;
	}
	while (node_idx != cInvalidNodeIndex);
}

void QuadTree::WidenAndUnlockNodeAndParents(uint32 inNodeIndex, const AABox &inNewBounds)
{
	uint32 node_idx = inNodeIndex;

	for (;;)
	{
		// Mark node as unlocked
		Node &node = mAllocator->Get(node_idx);
		node.mIsLocked = false;

		// Get our parent
		uint32 parent_idx = node.mParentNodeIndex;
		if (parent_idx == cInvalidNodeIndex)
			break;

		// Find which child of the parent we're in
		Node &parent_node = mAllocator->Get(parent_idx);
		NodeID node_id = NodeID::sFromNodeIndex(node_idx);
		int child_idx = -1;
		for (int i = 0; i < 4; ++i)
			if (parent_node.mChildNodeID[i] == node_id)
			{
				// Found one, set the node index and child index and update the bounding box too
				child_idx = i;
				break;
			}
		JPH_ASSERT(child_idx != -1, "Nodes don't get removed from the tree, we must have found it");

		// To avoid any race conditions with other threads we only enlarge bounding boxes
		if (!parent_node.EncapsulateChildBounds(child_idx, inNewBounds))
		{
			// No changes to bounding box, only unlocking remains to be done
			if (parent_node.mIsLocked)
				UnlockNodeAndParents(parent_idx);
			break; 
		}

		// Update node index
		node_idx = parent_idx;
	}
}

bool QuadTree::TryInsertLeaf(TrackingVector &ioTracking, int inNodeIndex, NodeID inLeafID, const AABox &inLeafBounds, int inLeafNumBodies)
{
	// Tentively assign the node as parent
	bool leaf_is_node = inLeafID.IsNode();
	if (leaf_is_node)
	{
		uint32 leaf_idx = inLeafID.GetNodeIndex();
		mAllocator->Get(leaf_idx).mParentNodeIndex = inNodeIndex;
	}

	// Fetch node that we're adding to
	Node &node = mAllocator->Get(inNodeIndex);

	// Find an empty child
	for (uint32 child_idx = 0; child_idx < 4; ++child_idx)
		if (node.mChildNodeID[child_idx].IsValid())
		{
			// Check if we can claim it
			if (node.mChildNodeID[child_idx].CompareExchange(NodeID::sInvalid(), inLeafID))
			{
				// We managed to add it to the node

				// If leaf was a body, we need to update its bookkeeping
				if (!leaf_is_node)
					SetBodyLocation(ioTracking, inLeafID.GetBodyID(), inNodeIndex, child_idx);

				// Now set the bounding box making the child valid for queries
				node.SetChildBounds(child_idx, inLeafBounds);

				// Widen the bounds for our parents too
				WidenAndUnlockNodeAndParents(inNodeIndex, inLeafBounds);

				// Update body counter
				mNumBodies += inLeafNumBodies;

				// And we're done
				return true;
			}
		}

	return false;
}

bool QuadTree::TryCreateNewRoot(TrackingVector &ioTracking, atomic<uint32> &ioRootNodeIndex, NodeID inLeafID, const AABox &inLeafBounds, int inLeafNumBodies)
{
	// Fetch old root
	uint32 root_idx = ioRootNodeIndex;
	Node &root = mAllocator->Get(root_idx);

	// Create new root
	uint32 new_root_idx = AllocateNode(false);
	Node &new_root = mAllocator->Get(new_root_idx);

	// First child is current root
	new_root.mChildNodeID[0] = NodeID::sFromNodeIndex(root_idx);
	AABox old_root_bounds;
	root.GetNodeBounds(old_root_bounds);
	new_root.SetChildBounds(0, old_root_bounds);

	// Second child is new leaf
	new_root.mChildNodeID[1] = inLeafID;
	new_root.SetChildBounds(1, inLeafBounds);

	// Tentatively assign new root as parent
	bool leaf_is_node = inLeafID.IsNode();
	if (leaf_is_node)
	{
		uint32 leaf_idx = inLeafID.GetNodeIndex();
		mAllocator->Get(leaf_idx).mParentNodeIndex = new_root_idx;
	}

	// Try to swap it
	if (ioRootNodeIndex.compare_exchange_strong(root_idx, new_root_idx))
	{
		// We managed to set the new root

		// If leaf was a body, we need to update its bookkeeping
		if (!leaf_is_node)
			SetBodyLocation(ioTracking, inLeafID.GetBodyID(), new_root_idx, 1);

		// Store parent node for old root
		root.mParentNodeIndex = new_root_idx;

		// If a bounds change was in progress while we were assigning the root it will not have updated the root volume (since it did not know the new root)
		// so we calculate the bounding box again and encapsulate it (to avoid conflicts with other threads which may adjust the old root bounds while we're reading them)
		root.GetNodeBounds(old_root_bounds);
		new_root.EncapsulateChildBounds(0, old_root_bounds);

		// Update body counter
		mNumBodies += inLeafNumBodies;

		// And we're done
		return true;
	}

	// Failed to swap, someone else must have created a new root, try again
	mAllocator->DestructObject(new_root_idx);
	return false;
}

void QuadTree::AddBodiesPrepare(const BodyVector &inBodies, TrackingVector &ioTracking, BodyID *ioBodyIDs, int inNumber, AddState &outState)
{
	// Assert sane input
	JPH_ASSERT(ioBodyIDs != nullptr);
	JPH_ASSERT(inNumber > 0);

#ifdef JPH_ENABLE_ASSERTS
	// Below we just cast the body ID's to node ID's, check here that that is valid
	for (const BodyID *b = ioBodyIDs, *b_end = ioBodyIDs + inNumber; b < b_end; ++b)
		NodeID::sFromBodyID(*b);
#endif

	// Build subtree for the new bodies
	outState.mLeafID = BuildTree(inBodies, ioTracking, (NodeID *)ioBodyIDs, inNumber, cInvalidNodeIndex, true, outState.mLeafBounds);

#ifdef _DEBUG
	if (outState.mLeafID.IsNode())
		ValidateTree(inBodies, ioTracking, outState.mLeafID.GetNodeIndex(), inNumber);
#endif
}

void QuadTree::AddBodiesFinalize(TrackingVector &ioTracking, int inNumberBodies, const AddState &inState)
{
	// Assert sane input
	JPH_ASSERT(inNumberBodies > 0);

	// Mark tree dirty
	mIsDirty = true;

	// Get the current root node
	RootNode &root_node = GetCurrentRoot();

	for (;;)
	{
		// Check if we can insert the body in the root
		if (TryInsertLeaf(ioTracking, root_node.mIndex, inState.mLeafID, inState.mLeafBounds, inNumberBodies))
			return;
		
		// Check if we can create a new root
		if (TryCreateNewRoot(ioTracking, root_node.mIndex, inState.mLeafID, inState.mLeafBounds, inNumberBodies))
			return;
	}
}

void QuadTree::AddBodiesAbort(TrackingVector &ioTracking, const AddState &inState)
{
	// Collect all bodies
	Allocator::Batch free_batch;
	NodeID node_stack[cStackSize];
	node_stack[0] = inState.mLeafID;
	JPH_ASSERT(node_stack[0].IsValid());
	int top = 0;
	do
	{
		// Check if node is a body
		NodeID child_node_id = node_stack[top];
		if (child_node_id.IsBody())
		{
			// Reset location of body
			InvalidateBodyLocation(ioTracking, child_node_id.GetBodyID());
		}
		else
		{
			// Process normal node
			uint32 node_idx = child_node_id.GetNodeIndex();
			const Node &node = mAllocator->Get(node_idx);
			for (int i = 0; i < 4; ++i)
			{
				NodeID sub_child_node_id = node.mChildNodeID[i];
				if (sub_child_node_id.IsValid())
				{
					JPH_ASSERT(top < cStackSize);
					node_stack[top] = sub_child_node_id;
					top++;
				}
			}

			// Mark it to be freed
			mAllocator->AddObjectToBatch(free_batch, node_idx);
		}
		--top;
	} 
	while (top >= 0);

	// Now free all nodes as a single batch
	mAllocator->DestructObjectBatch(free_batch);
}

void QuadTree::RemoveBodies([[maybe_unused]] const BodyVector &inBodies, TrackingVector &ioTracking, BodyID *ioBodyIDs, int inNumber)
{
	// Assert sane input
	JPH_ASSERT(ioBodyIDs != nullptr);
	JPH_ASSERT(inNumber > 0);

	// Mark tree dirty
	mIsDirty = true;

	for (BodyID *cur = ioBodyIDs, *end = ioBodyIDs + inNumber; cur < end; ++cur)
	{
		// Check if BodyID is correct
		JPH_ASSERT(inBodies[cur->GetIndex()]->GetID() == *cur, "Provided BodyID doesn't match BodyID in body manager");

		// Get location of body
		uint32 node_idx, child_idx;
		GetBodyLocation(ioTracking, *cur, node_idx, child_idx);

		// First we reset our internal bookkeeping
		InvalidateBodyLocation(ioTracking, *cur);

		// Then we make the bounding box invalid, no queries can find this node anymore
		Node &node = mAllocator->Get(node_idx);
		node.InvalidateChildBounds(child_idx);

		// Finally we reset the child id, this makes the node available for adds again
		node.mChildNodeID[child_idx] = NodeID::sInvalid();

		// We don't need to bubble up our bounding box changes to our parents since we never make volumes smaller, only bigger
		// But we do need to unlock the nodes so that the tree can be rebuilt
		UnlockNodeAndParents(node_idx);
	}

	mNumBodies -= inNumber;
}

void QuadTree::NotifyBodiesAABBChanged(const BodyVector &inBodies, const TrackingVector &inTracking, BodyID *ioBodyIDs, int inNumber)
{
	// Assert sane input
	JPH_ASSERT(ioBodyIDs != nullptr);
	JPH_ASSERT(inNumber > 0);

	for (BodyID *cur = ioBodyIDs, *end = ioBodyIDs + inNumber; cur < end; ++cur)
	{
		// Check if BodyID is correct
		const Body *body = inBodies[cur->GetIndex()];
		JPH_ASSERT(body->GetID() == *cur, "Provided BodyID doesn't match BodyID in body manager");

		// Get the new bounding box
		const AABox &new_bounds = body->GetWorldSpaceBounds();

		// Get location of body
		uint32 node_idx, child_idx;
		GetBodyLocation(inTracking, *cur, node_idx, child_idx);

		// Widen bounds for node
		Node &node = mAllocator->Get(node_idx);
		if (node.EncapsulateChildBounds(child_idx, new_bounds))
		{
			// Mark tree dirty
			mIsDirty = true;

			// If bounds changed, widen the bounds for our parents too
			WidenAndUnlockNodeAndParents(node_idx, new_bounds);
		}
	}
}

template <class Visitor>
JPH_INLINE void QuadTree::WalkTree(const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking, Visitor &ioVisitor JPH_IF_TRACK_BROADPHASE_STATS(, LayerToStats &ioStats)) const
{
	// Get the root
	const RootNode &root_node = GetCurrentRoot();

#ifdef JPH_TRACK_BROADPHASE_STATS
	// Start tracking stats
	int bodies_visited = 0;
	int hits_collected = 0;
	int nodes_visited = 0;
	uint64 collector_ticks = 0;

	uint64 start = GetProcessorTickCount();
#endif // JPH_TRACK_BROADPHASE_STATS

	NodeID node_stack[cStackSize];
	node_stack[0] = root_node.GetNodeID();
	int top = 0;
	do
	{
		// Check if node is a body
		NodeID child_node_id = node_stack[top];
		if (child_node_id.IsBody())
		{
			// Track amount of bodies visited
			JPH_IF_TRACK_BROADPHASE_STATS(++bodies_visited;)

			BodyID body_id = child_node_id.GetBodyID();
			ObjectLayer object_layer = inTracking[body_id.GetIndex()].mObjectLayer; // We're not taking a lock on the body, so it may be in the process of being removed so check if the object layer is invalid
			if (object_layer != cObjectLayerInvalid && inObjectLayerFilter.ShouldCollide(object_layer))
			{
				JPH_PROFILE("VisitBody");

				// Track amount of hits
				JPH_IF_TRACK_BROADPHASE_STATS(++hits_collected;)

				// Start track time the collector takes
				JPH_IF_TRACK_BROADPHASE_STATS(uint64 collector_start = GetProcessorTickCount();)

				// We found a body we collide with, call our visitor
				ioVisitor.VisitBody(body_id, top);

				// End track time the collector takes
				JPH_IF_TRACK_BROADPHASE_STATS(collector_ticks += GetProcessorTickCount() - collector_start;)

				// Check if we're done
				if (ioVisitor.ShouldAbort())
					break;
			}
		}
		else if (child_node_id.IsValid())
		{
			JPH_IF_TRACK_BROADPHASE_STATS(++nodes_visited;)

			// Process normal node
			const Node &node = mAllocator->Get(child_node_id.GetNodeIndex());
			JPH_ASSERT(IsAligned(&node, JPH_CACHE_LINE_SIZE));

			// Load bounds of 4 children
			Vec4 bounds_minx = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinX);
			Vec4 bounds_miny = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinY);
			Vec4 bounds_minz = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinZ);
			Vec4 bounds_maxx = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxX);
			Vec4 bounds_maxy = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxY);
			Vec4 bounds_maxz = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxZ);

			// Load ids for 4 children
			UVec4 child_ids = UVec4::sLoadInt4Aligned((const uint32 *)&node.mChildNodeID[0]);

			// Check which sub nodes to visit
			int num_results = ioVisitor.VisitNodes(bounds_minx, bounds_miny, bounds_minz, bounds_maxx, bounds_maxy, bounds_maxz, child_ids, top);
			if (num_results > 0)
			{
				// Push them onto the stack
				if (top + 4 < cStackSize)
				{
					child_ids.StoreInt4((uint32 *)&node_stack[top]);
					top += num_results;
				}
				else
					JPH_ASSERT(false, "Stack full!");
			}
		}

		// Fetch next node until we find one that the visitor wants to see
		do 
			--top;
		while (top >= 0 && !ioVisitor.ShouldVisitNode(top));
	} 
	while (top >= 0);

#ifdef JPH_TRACK_BROADPHASE_STATS
	// Calculate total time the broadphase walk took
	uint64 total_ticks = GetProcessorTickCount() - start;

	// Update stats under lock protection (slow!)
	{
		unique_lock lock(mStatsMutex);
		Stat &s = ioStats[inObjectLayerFilter.GetDescription()];
		s.mNumQueries++;
		s.mNodesVisited += nodes_visited;
		s.mBodiesVisited += bodies_visited;
		s.mHitsReported += hits_collected;
		s.mTotalTicks += total_ticks;
		s.mCollectorTicks += collector_ticks;
	}
#endif // JPH_TRACK_BROADPHASE_STATS
}

void QuadTree::CastRay(const RayCast &inRay, RayCastBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE				Visitor(const RayCast &inRay, RayCastBodyCollector &ioCollector) :
			mOrigin(inRay.mOrigin),
			mInvDirection(inRay.mDirection),
			mCollector(ioCollector)
		{
			mFractionStack[0] = -1;
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool			ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool			ShouldVisitNode(int inStackTop) const
		{
			return mFractionStack[inStackTop] < mCollector.GetEarlyOutFraction();
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int			VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop)
		{
			// Test the ray against 4 bounding boxes
			Vec4 fraction = RayAABox4(mOrigin, mInvDirection, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Count how many results are hitting
			UVec4 hitting = Vec4::sLess(fraction, Vec4::sReplicate(mCollector.GetEarlyOutFraction()));
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
				Vec4::sSort4Reverse(fraction, ioChildNodeIDs);

				// Shift the results so that only the hitting ones remain
				ioChildNodeIDs = ioChildNodeIDs.ShiftComponents4Minus(num_results);
				fraction = fraction.ReinterpretAsInt().ShiftComponents4Minus(num_results).ReinterpretAsFloat();

				// Push them onto the stack
				if (inStackTop + 4 < cStackSize)
					fraction.StoreFloat4((Float4 *)&mFractionStack[inStackTop]);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void			VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			BroadPhaseCastResult result { inBodyID, mFractionStack[inStackTop] };
			mCollector.AddHit(result);
		}

	private:
		Vec3					mOrigin;
		RayInvDirection			mInvDirection;
		RayCastBodyCollector &	mCollector;
		float					mFractionStack[cStackSize];
	};

	Visitor visitor(inRay, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCastRayStats));
}

void QuadTree::CollideAABox(const AABox &inBox, CollideShapeBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE					Visitor(const AABox &inBox, CollideShapeBodyCollector &ioCollector) :
			mBox(inBox),
			mCollector(ioCollector)
		{
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool				ShouldVisitNode(int inStackTop) const
		{
			return true;
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int				VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop) const
		{
			// Test the box vs 4 boxes
			UVec4 hitting = AABox4VsBox(mBox, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Count how many results are hitting
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort trues first
				UVec4::sSort4True(hitting, ioChildNodeIDs);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void				VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			mCollector.AddHit(inBodyID);
		}

	private:
		const AABox &				mBox;
		CollideShapeBodyCollector &	mCollector;
	};

	Visitor visitor(inBox, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCollideAABoxStats));
}

void QuadTree::CollideSphere(Vec3Arg inCenter, float inRadius, CollideShapeBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE					Visitor(Vec3Arg inCenter, float inRadius, CollideShapeBodyCollector &ioCollector) :
			mCenterX(inCenter.SplatX()),
			mCenterY(inCenter.SplatY()),
			mCenterZ(inCenter.SplatZ()),
			mRadiusSq(Vec4::sReplicate(Square(inRadius))),
			mCollector(ioCollector)
		{
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool				ShouldVisitNode(int inStackTop) const
		{
			return true;
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int				VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop) const
		{
			// Get closest point on box
			Vec4 closest_x = Vec4::sMin(Vec4::sMax(mCenterX, inBoundsMinX), inBoundsMaxX);
			Vec4 closest_y = Vec4::sMin(Vec4::sMax(mCenterY, inBoundsMinY), inBoundsMaxY);
			Vec4 closest_z = Vec4::sMin(Vec4::sMax(mCenterZ, inBoundsMinZ), inBoundsMaxZ);

			// Test the distance from the center of the sphere to the box is smaller than the radius
			Vec4 distance_sq = Square(closest_x - mCenterX) + Square(closest_y - mCenterY) + Square(closest_z - mCenterZ);
			UVec4 hitting = Vec4::sLessOrEqual(distance_sq, mRadiusSq);

			// Count how many results are hitting
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort trues first
				UVec4::sSort4True(hitting, ioChildNodeIDs);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void				VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			mCollector.AddHit(inBodyID);
		}

	private:
		Vec4						mCenterX;
		Vec4						mCenterY;
		Vec4						mCenterZ;
		Vec4						mRadiusSq;
		CollideShapeBodyCollector &	mCollector;
	};

	Visitor visitor(inCenter, inRadius, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCollideSphereStats));
}

void QuadTree::CollidePoint(Vec3Arg inPoint, CollideShapeBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE					Visitor(Vec3Arg inPoint, CollideShapeBodyCollector &ioCollector) :
			mPoint(inPoint),
			mCollector(ioCollector)
		{
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool				ShouldVisitNode(int inStackTop) const
		{
			return true;
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int				VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop) const
		{
			// Test if point overlaps with box
			UVec4 hitting = AABox4VsPoint(mPoint, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Count how many results are hitting
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort trues first
				UVec4::sSort4True(hitting, ioChildNodeIDs);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void				VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			mCollector.AddHit(inBodyID);
		}

	private:
		Vec3						mPoint;
		CollideShapeBodyCollector &	mCollector;
	};

	Visitor visitor(inPoint, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCollidePointStats));
}

void QuadTree::CollideOrientedBox(const OrientedBox &inBox, CollideShapeBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE					Visitor(const OrientedBox &inBox, CollideShapeBodyCollector &ioCollector) :
			mBox(inBox),
			mCollector(ioCollector)
		{
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool				ShouldVisitNode(int inStackTop) const
		{
			return true;
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int				VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop) const
		{
			// Test if point overlaps with box
			UVec4 hitting = AABox4VsBox(mBox, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Count how many results are hitting
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort trues first
				UVec4::sSort4True(hitting, ioChildNodeIDs);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void				VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			mCollector.AddHit(inBodyID);
		}

	private:
		OrientedBox					mBox;
		CollideShapeBodyCollector &	mCollector;
	};

	Visitor visitor(inBox, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCollideOrientedBoxStats));
}

void QuadTree::CastAABox(const AABoxCast &inBox, CastShapeBodyCollector &ioCollector, const ObjectLayerFilter &inObjectLayerFilter, const TrackingVector &inTracking) const
{
	class Visitor
	{
	public:
		/// Constructor
		JPH_INLINE					Visitor(const AABoxCast &inBox, CastShapeBodyCollector &ioCollector) :
			mOrigin(inBox.mBox.GetCenter()),
			mExtent(inBox.mBox.GetExtent()),
			mInvDirection(inBox.mDirection),
			mCollector(ioCollector)
		{
			mFractionStack[0] = -1;
		}

		/// Returns true if further processing of the tree should be aborted
		JPH_INLINE bool				ShouldAbort() const
		{
			return mCollector.ShouldEarlyOut();
		}

		/// Returns true if this node / body should be visited, false if no hit can be generated
		JPH_INLINE bool				ShouldVisitNode(int inStackTop) const
		{
			return mFractionStack[inStackTop] < mCollector.GetEarlyOutFraction();
		}

		/// Visit nodes, returns number of hits found and sorts ioChildNodeIDs so that they are at the beginning of the vector.
		JPH_INLINE int				VisitNodes(Vec4Arg inBoundsMinX, Vec4Arg inBoundsMinY, Vec4Arg inBoundsMinZ, Vec4Arg inBoundsMaxX, Vec4Arg inBoundsMaxY, Vec4Arg inBoundsMaxZ, UVec4 &ioChildNodeIDs, int inStackTop)
		{
			// Enlarge them by the casted aabox extents
			AABox4EnlargeWithExtent(mExtent, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Test 4 children
			Vec4 fraction = RayAABox4(mOrigin, mInvDirection, inBoundsMinX, inBoundsMinY, inBoundsMinZ, inBoundsMaxX, inBoundsMaxY, inBoundsMaxZ);

			// Count how many results are hitting
			UVec4 hitting = Vec4::sLess(fraction, Vec4::sReplicate(mCollector.GetEarlyOutFraction()));
			int num_results = hitting.CountTrues();
			if (num_results > 0)
			{
				// Sort so that highest values are first (we want to first process closer hits and we process stack top to bottom)
				Vec4::sSort4Reverse(fraction, ioChildNodeIDs);

				// Shift the results so that only the hitting ones remain
				ioChildNodeIDs = ioChildNodeIDs.ShiftComponents4Minus(num_results);
				fraction = fraction.ReinterpretAsInt().ShiftComponents4Minus(num_results).ReinterpretAsFloat();

				// Push them onto the stack
				if (inStackTop + 4 < cStackSize)
					fraction.StoreFloat4((Float4 *)&mFractionStack[inStackTop]);
			}

			return num_results;
		}

		/// Visit a body, returns false if the algorithm should terminate because no hits can be generated anymore
		JPH_INLINE void				VisitBody(const BodyID &inBodyID, int inStackTop)
		{
			// Store potential hit with body
			BroadPhaseCastResult result { inBodyID, mFractionStack[inStackTop] };
			mCollector.AddHit(result);
		}

	private:
		Vec3						mOrigin;
		Vec3						mExtent;
		RayInvDirection				mInvDirection;
		CastShapeBodyCollector &	mCollector;
		float						mFractionStack[cStackSize];
	};

	Visitor visitor(inBox, ioCollector);
	WalkTree(inObjectLayerFilter, inTracking, visitor JPH_IF_TRACK_BROADPHASE_STATS(, mCastAABoxStats));
}

void QuadTree::FindCollidingPairs(const BodyVector &inBodies, const BodyID *inActiveBodies, int inNumActiveBodies, float inSpeculativeContactDistance, BodyPairCollector &ioPairCollector, ObjectLayerPairFilter inObjectLayerPairFilter) const
{
	// Note that we don't lock the tree at this point. We know that the tree is not going to be swapped or deleted while finding collision pairs due to the way the jobs are scheduled in the PhysicsSystem::Update.
	// We double check this at the end of the function.
	const RootNode &root_node = GetCurrentRoot();
	JPH_ASSERT(root_node.mIndex != cInvalidNodeIndex);

	// Assert sane input
	JPH_ASSERT(inActiveBodies != nullptr);
	JPH_ASSERT(inNumActiveBodies > 0);
		
	NodeID node_stack[cStackSize];

	// Loop over all active bodies
	for (int b1 = 0; b1 < inNumActiveBodies; ++b1)
	{
		BodyID b1_id = inActiveBodies[b1];
		const Body &body1 = *inBodies[b1_id.GetIndex()];
		JPH_ASSERT(!body1.IsStatic());

		// Expand the bounding box by the speculative contact distance
		AABox bounds1 = body1.GetWorldSpaceBounds();
		bounds1.ExpandBy(Vec3::sReplicate(inSpeculativeContactDistance));

		// Test each body with the tree
		node_stack[0] = root_node.GetNodeID();
		int top = 0;
		do
		{
			// Check if node is a body
			NodeID child_node_id = node_stack[top];
			if (child_node_id.IsBody())
			{
				// Don't collide with self
				BodyID b2_id = child_node_id.GetBodyID();
				if (b1_id != b2_id)
				{			
					// Collision between dynamic pairs need to be picked up only once
					const Body &body2 = *inBodies[b2_id.GetIndex()];
					if (inObjectLayerPairFilter(body1.GetObjectLayer(), body2.GetObjectLayer())
						&& Body::sFindCollidingPairsCanCollide(body1, body2))
					{
						// In the broadphase we widen the bounding box when a body moves, do a final
						// check to see if the bounding boxes actually overlap
						if (bounds1.Overlaps(body2.GetWorldSpaceBounds()))
						{
							// Store potential hit between bodies
							ioPairCollector.AddHit({ b1_id, b2_id });
						}
					}
				}
			}
			else if (child_node_id.IsValid())
			{
				// Process normal node
				const Node &node = mAllocator->Get(child_node_id.GetNodeIndex());
				JPH_ASSERT(IsAligned(&node, JPH_CACHE_LINE_SIZE));

				// Get bounds of 4 children
				Vec4 bounds_minx = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinX);
				Vec4 bounds_miny = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinY);
				Vec4 bounds_minz = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMinZ);
				Vec4 bounds_maxx = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxX);
				Vec4 bounds_maxy = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxY);
				Vec4 bounds_maxz = Vec4::sLoadFloat4Aligned((const Float4 *)&node.mBoundsMaxZ);

				// Test overlap
				UVec4 overlap = AABox4VsBox(bounds1, bounds_minx, bounds_miny, bounds_minz, bounds_maxx, bounds_maxy, bounds_maxz);
				int num_results = overlap.CountTrues();
				if (num_results > 0)
				{
					// Load ids for 4 children
					UVec4 child_ids = UVec4::sLoadInt4Aligned((const uint32 *)&node.mChildNodeID[0]);

					// Sort so that overlaps are first
					UVec4::sSort4True(overlap, child_ids);

					// Push them onto the stack
					if (top + 4 < cStackSize)
					{
						child_ids.StoreInt4((uint32 *)&node_stack[top]);
						top += num_results;
					}
					else
						JPH_ASSERT(false, "Stack full!");
				}
			}
			--top;
		} 
		while (top >= 0);
	}

	// Test that the root node was not swapped while finding collision pairs. 
	// This would mean that UpdateFinalize/DiscardOldTree ran during collision detection which should not be possible due to the way the jobs are scheduled.
	JPH_ASSERT(root_node.mIndex != cInvalidNodeIndex);
	JPH_ASSERT(&root_node == &GetCurrentRoot());
}

#ifdef _DEBUG

void QuadTree::ValidateTree(const BodyVector &inBodies, const TrackingVector &inTracking, uint32 inNodeIndex, uint32 inNumExpectedBodies) const
{
	JPH_PROFILE_FUNCTION();

	// Root should be valid
	JPH_ASSERT(inNodeIndex != cInvalidNodeIndex);

	// To avoid call overhead, create a stack in place
	struct StackEntry
	{
		uint32			mNodeIndex;
		uint32			mParentNodeIndex;
	};
	StackEntry stack[cStackSize];
	stack[0].mNodeIndex = inNodeIndex;
	stack[0].mParentNodeIndex = cInvalidNodeIndex;
	int top = 0;

	uint32 num_bodies = 0;

	do
	{
		// Copy entry from the stack
		StackEntry cur_stack = stack[top];

		// Validate parent
		const Node &node = mAllocator->Get(cur_stack.mNodeIndex);
		JPH_ASSERT(node.mParentNodeIndex == cur_stack.mParentNodeIndex);

		// Validate that when a parent is locked that all of its children are also
		JPH_ASSERT(cur_stack.mParentNodeIndex == cInvalidNodeIndex || !mAllocator->Get(cur_stack.mParentNodeIndex).mIsLocked || node.mIsLocked);

		// Loop children
		for (uint32 i = 0; i < 4; ++i)
		{
			NodeID child_node_id = node.mChildNodeID[i];
			if (child_node_id.IsValid())
			{
				if (child_node_id.IsNode())
				{
					// Child is a node, recurse
					uint32 child_idx = child_node_id.GetNodeIndex();
					StackEntry &new_entry = stack[top++];
					new_entry.mNodeIndex = child_idx;
					new_entry.mParentNodeIndex = cur_stack.mNodeIndex;

					// Validate that the bounding box is bigger or equal to the bounds in the tree
					// Bounding box could also be invalid if all children of our child were removed
					AABox child_bounds;
					node.GetChildBounds(i, child_bounds);
					AABox real_child_bounds;
					mAllocator->Get(child_idx).GetNodeBounds(real_child_bounds);
					JPH_ASSERT(child_bounds.Contains(real_child_bounds) || !real_child_bounds.IsValid());
				}
				else
				{
					// Increment number of bodies found
					++num_bodies;

					// Check if tracker matches position of body
					uint32 node_idx, child_idx;
					GetBodyLocation(inTracking, child_node_id.GetBodyID(), node_idx, child_idx);
					JPH_ASSERT(node_idx == cur_stack.mNodeIndex);
					JPH_ASSERT(child_idx == i);

					// Validate that the body bounds are bigger or equal to the bounds in the tree
					AABox body_bounds;
					node.GetChildBounds(i, body_bounds);
					const Body *body = inBodies[child_node_id.GetBodyID().GetIndex()];
					AABox cached_body_bounds = body->GetWorldSpaceBounds();
					AABox real_body_bounds = body->GetShape()->GetWorldSpaceBounds(body->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f));
					JPH_ASSERT(cached_body_bounds == real_body_bounds); // Check that cached body bounds are up to date
					JPH_ASSERT(body_bounds.Contains(real_body_bounds));
				}
			}
		}
		--top;
	}
	while (top >= 0);

	// Check that the amount of bodies in the tree matches our counter
	JPH_ASSERT(num_bodies == inNumExpectedBodies);
}

#endif

#ifdef JPH_TRACK_BROADPHASE_STATS

void QuadTree::ReportStats(const char *inName, const LayerToStats &inLayer) const
{
	uint64 ticks_per_sec = GetProcessorTicksPerSecond();

	for (const LayerToStats::value_type &kv : inLayer)
	{
		double total_time = 1000.0 * double(kv.second.mTotalTicks) / double(ticks_per_sec);
		double total_time_excl_collector = 1000.0 * double(kv.second.mTotalTicks - kv.second.mCollectorTicks) / double(ticks_per_sec);
		double hits_reported_vs_bodies_visited = kv.second.mBodiesVisited > 0? 100.0 * double(kv.second.mHitsReported) / double(kv.second.mBodiesVisited) : 100.0;
		double hits_reported_vs_nodes_visited = kv.second.mNodesVisited > 0? double(kv.second.mHitsReported) / double(kv.second.mNodesVisited) : -1.0f;

		stringstream str;
		str << inName << ", " << kv.first << ", " << mName << ", " << kv.second.mNumQueries << ", " << total_time << ", " << total_time_excl_collector << ", " << kv.second.mNodesVisited << ", " << kv.second.mBodiesVisited << ", " << kv.second.mHitsReported << ", " << hits_reported_vs_bodies_visited << ", " << hits_reported_vs_nodes_visited;
		Trace(str.str().c_str());
	}
}

void QuadTree::ReportStats() const
{
	unique_lock lock(mStatsMutex);
	ReportStats("RayCast", mCastRayStats);
	ReportStats("CollideAABox", mCollideAABoxStats);
	ReportStats("CollideSphere", mCollideSphereStats);
	ReportStats("CollidePoint", mCollidePointStats);
	ReportStats("CollideOrientedBox", mCollideOrientedBoxStats);
	ReportStats("CastAABox", mCastAABoxStats);
}

#endif // JPH_TRACK_BROADPHASE_STATS

} // JPH