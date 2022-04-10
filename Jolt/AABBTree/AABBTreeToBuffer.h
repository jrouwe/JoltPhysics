// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/AABBTree/AABBTreeBuilder.h>
#include <Jolt/Core/ByteBuffer.h>
#include <Jolt/Geometry/IndexedTriangle.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <deque>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

/// How the tree should be converted
enum class EAABBTreeToBufferConvertMode
{
	DepthFirst,							///< Arrange the nodes depth first, put the triangles right after the leaf nodes (so interleaving them with nodes)
	DepthFirstTrianglesLast,			///< Arrange the nodes depth first and put all triangles blocks after the last node block
	BreadthFirst,						///< Arrange the nodes breadth first, put the triangles right after the leaf nodes (so interleaving them with nodes)
	BreadthFirstTrianglesLast,			///< Arrange the nodes breadth first and put all triangles blocks after the last node block
};

/// Convert mode to string
inline string ConvertToString(EAABBTreeToBufferConvertMode inConvertMode)
{
	switch (inConvertMode)
	{
	case EAABBTreeToBufferConvertMode::DepthFirst:					return "DepthFirst";
	case EAABBTreeToBufferConvertMode::DepthFirstTrianglesLast:		return "DepthFirstTrianglesLast";
	case EAABBTreeToBufferConvertMode::BreadthFirst:				return "BreadthFirst";
	case EAABBTreeToBufferConvertMode::BreadthFirstTrianglesLast:	return "BreadthFirstTrianglesLast";
	}

	JPH_ASSERT(false);
	return "Invalid";
}

/// Struct that holds statistics about the AABB tree that was built
struct AABBTreeToBufferStats
{
	uint			mTotalSize = 0;									///< Total size of the built tree in bytes
	uint			mNodesSize = 0;									///< Total size of all nodes in the tree in bytes
	uint			mTrianglesSize = 0;								///< Total size of all triangles in the tree in bytes
	float			mBytesPerTriangle = 0.0f;						///< Average number of bytes per triangle (includes all tree overhead)
	string			mTriangleCodecName;								///< Name of the codec that was used to build the tree
	float			mVerticesPerTriangle = 0.0f;					///< How many vertices a triangle on average has
};
		
/// Conversion algorithm that converts an AABB tree to an optimized binary buffer
template <class TriangleCodec, class NodeCodec>
class AABBTreeToBuffer
{
public:
	/// Header for the tree
	using NodeHeader = typename NodeCodec::Header;

	/// Size in bytes of the header of the tree
	static const int HeaderSize = NodeCodec::HeaderSize;

	/// Maximum number of children per node in the tree
	static const int NumChildrenPerNode = NodeCodec::NumChildrenPerNode;

	/// Header for the triangles
	using TriangleHeader = typename TriangleCodec::TriangleHeader;

	/// Size in bytes of the header for the triangles
	static const int TriangleHeaderSize = TriangleCodec::TriangleHeaderSize;

	/// Convert AABB tree. Returns false if failed.
	bool							Convert(const VertexList &inVertices, const AABBTreeBuilder::Node *inRoot, AABBTreeToBufferStats &outStats, string &outError, EAABBTreeToBufferConvertMode inConvertMode = EAABBTreeToBufferConvertMode::DepthFirst)
	{
		const typename NodeCodec::EncodingContext node_ctx;
		typename TriangleCodec::EncodingContext tri_ctx;

		// Estimate the amount of memory required
		uint tri_count = inRoot->GetTriangleCountInTree();
		uint node_count = inRoot->GetNodeCount();
		uint nodes_size = node_ctx.GetPessimisticMemoryEstimate(node_count);
		uint total_size = HeaderSize + TriangleHeaderSize + nodes_size + tri_ctx.GetPessimisticMemoryEstimate(tri_count);
		mTree.reserve(total_size);

		// Reset counters
		mNodesSize = 0;

		// Add headers
		NodeHeader *header = HeaderSize > 0? mTree.Allocate<NodeHeader>() : nullptr;
		TriangleHeader *triangle_header = TriangleHeaderSize > 0? mTree.Allocate<TriangleHeader>() : nullptr;

		struct NodeData
		{
			const AABBTreeBuilder::Node *	mNode = nullptr;							// Node that this entry belongs to
			Vec3							mNodeBoundsMin;								// Quantized node bounds
			Vec3							mNodeBoundsMax;
			uint							mNodeStart = uint(-1);						// Start of node in mTree
			uint							mTriangleStart = uint(-1);					// Start of the triangle data in mTree
			uint							mNumChildren = 0;							// Number of children
			uint							mChildNodeStart[NumChildrenPerNode];		// Start of the children of the node in mTree
			uint							mChildTrianglesStart[NumChildrenPerNode];	// Start of the triangle data in mTree
			uint *							mParentChildNodeStart = nullptr;			// Where to store mNodeStart (to patch mChildNodeStart of my parent)
			uint *							mParentTrianglesStart = nullptr;			// Where to store mTriangleStart (to patch mChildTrianglesStart of my parent)
		};
		
		deque<NodeData *> to_process;
		deque<NodeData *> to_process_triangles;
		vector<NodeData> node_list;

		node_list.reserve(node_count); // Needed to ensure that array is not reallocated, so we can keep pointers in the array
		
		NodeData root;
		root.mNode = inRoot;
		root.mNodeBoundsMin = inRoot->mBounds.mMin;
		root.mNodeBoundsMax = inRoot->mBounds.mMax;
		node_list.push_back(root);
		to_process.push_back(&node_list.back());

		// Child nodes out of loop so we don't constantly realloc it
		vector<const AABBTreeBuilder::Node *> child_nodes;
		child_nodes.reserve(NumChildrenPerNode);

		for (;;)
		{
			while (!to_process.empty())
			{
				// Get the next node to process
				NodeData *node_data = nullptr;
				switch (inConvertMode)
				{
				case EAABBTreeToBufferConvertMode::DepthFirst:
				case EAABBTreeToBufferConvertMode::DepthFirstTrianglesLast:
					node_data = to_process.back();
					to_process.pop_back();
					break;

				case EAABBTreeToBufferConvertMode::BreadthFirst:
				case EAABBTreeToBufferConvertMode::BreadthFirstTrianglesLast:
					node_data = to_process.front();
					to_process.pop_front();
					break;

				default:
					JPH_ASSERT(false);
					break;
				}

				// Due to quantization box could have become bigger, not smaller
				JPH_ASSERT(AABox(node_data->mNodeBoundsMin, node_data->mNodeBoundsMax).Contains(node_data->mNode->mBounds), "AABBTreeToBuffer: Bounding box became smaller!");

				// Collect the first NumChildrenPerNode sub-nodes in the tree
				child_nodes.clear(); // Won't free the memory
				node_data->mNode->GetNChildren(NumChildrenPerNode, child_nodes);
				node_data->mNumChildren = (uint)child_nodes.size();

				// Fill in default child bounds
				Vec3 child_bounds_min[NumChildrenPerNode], child_bounds_max[NumChildrenPerNode];
				for (size_t i = 0; i < NumChildrenPerNode; ++i)
					if (i < child_nodes.size())
					{
						child_bounds_min[i] = child_nodes[i]->mBounds.mMin;
						child_bounds_max[i] = child_nodes[i]->mBounds.mMax;
					}
					else
					{
						child_bounds_min[i] = Vec3::sZero();
						child_bounds_max[i] = Vec3::sZero();
					}

				// Start a new node
				uint old_size = (uint)mTree.size();
				node_data->mNodeStart = node_ctx.NodeAllocate(node_data->mNode, node_data->mNodeBoundsMin, node_data->mNodeBoundsMax, child_nodes, child_bounds_min, child_bounds_max, mTree, outError);
				if (node_data->mNodeStart == uint(-1))
					return false;
				mNodesSize += (uint)mTree.size() - old_size;

				if (node_data->mNode->HasChildren())
				{
					for (size_t i = 0; i < child_nodes.size(); ++i)
					{
						// Depth first: Insert in reverse order so we process left child first when taking nodes from the back
						size_t idx = (inConvertMode == EAABBTreeToBufferConvertMode::DepthFirst || inConvertMode == EAABBTreeToBufferConvertMode::DepthFirstTrianglesLast)? 
							child_nodes.size() - 1 - i : i;
					
						// Due to quantization box could have become bigger, not smaller
						JPH_ASSERT(AABox(child_bounds_min[idx], child_bounds_max[idx]).Contains(child_nodes[idx]->mBounds), "AABBTreeToBuffer: Bounding box became smaller!");

						// Add child to list of nodes to be processed
						NodeData child;
						child.mNode = child_nodes[idx];
						child.mNodeBoundsMin = child_bounds_min[idx];
						child.mNodeBoundsMax = child_bounds_max[idx];
						child.mParentChildNodeStart = &node_data->mChildNodeStart[idx];
						child.mParentTrianglesStart = &node_data->mChildTrianglesStart[idx];
						NodeData *old = &node_list[0];
						node_list.push_back(child);
						if (old != &node_list[0])
						{
							outError = "Internal Error: Array reallocated, memory corruption!";
							return false;
						}

						switch (inConvertMode)
						{
						case EAABBTreeToBufferConvertMode::DepthFirst:
						case EAABBTreeToBufferConvertMode::BreadthFirst:
							to_process.push_back(&node_list.back());
							break;

						case EAABBTreeToBufferConvertMode::DepthFirstTrianglesLast:
						case EAABBTreeToBufferConvertMode::BreadthFirstTrianglesLast:
							if (node_list.back().mNode->HasChildren())
								to_process.push_back(&node_list.back());
							else
								to_process_triangles.push_back(&node_list.back());
							break;
						}
					}
				}
				else
				{				
					// Add triangles
					node_data->mTriangleStart = tri_ctx.Pack(inVertices, node_data->mNode->mTriangles, mTree, outError);
					if (node_data->mTriangleStart == uint(-1))
						return false;
				}

				// Patch offset into parent
				if (node_data->mParentChildNodeStart != nullptr)
				{
					*node_data->mParentChildNodeStart = node_data->mNodeStart;
					*node_data->mParentTrianglesStart = node_data->mTriangleStart;
				}
			}

			// If we've got triangles to process, loop again with just the triangles
			if (to_process_triangles.empty())
				break;
			else
				to_process.swap(to_process_triangles);
		}
		
		// Finalize all nodes
		for (NodeData &n : node_list)
			if (!node_ctx.NodeFinalize(n.mNode, n.mNodeStart, n.mNumChildren, n.mChildNodeStart, n.mChildTrianglesStart, mTree, outError))
				return false;
		
		// Finalize the triangles
		tri_ctx.Finalize(triangle_header, mTree);

		// Get stats
		tri_ctx.GetStats(outStats.mTriangleCodecName, outStats.mVerticesPerTriangle);

		// Validate that we reserved enough memory
		if (nodes_size < mNodesSize)
		{
			outError = "Internal Error: Not enough memory reserved for nodes!";
			return false;
		}
		if (total_size < (uint)mTree.size())
		{
			outError = "Internal Error: Not enough memory reserved for triangles!";
			return false;
		}

		// Finalize the nodes
		if (!node_ctx.Finalize(header, inRoot, node_list[0].mNodeStart, node_list[0].mTriangleStart, outError))
			return false;

		// Shrink the tree, this will invalidate the header and triangle_header variables
		mTree.shrink_to_fit();

		// Output stats
		outStats.mTotalSize = (uint)mTree.size();
		outStats.mNodesSize = mNodesSize;
		outStats.mTrianglesSize = (uint)mTree.size() - mNodesSize;
		outStats.mBytesPerTriangle = (float)mTree.size() / tri_count;

		return true;
	}

	/// Get resulting data
	inline const ByteBuffer &		GetBuffer() const
	{
		return mTree;
	}

	/// Get resulting data
	inline ByteBuffer &				GetBuffer()
	{
		return mTree;
	}

	/// Get header for tree
	inline const NodeHeader *		GetNodeHeader() const
	{
		return mTree.Get<NodeHeader>(0);
	}

	/// Get header for triangles
	inline const TriangleHeader *	GetTriangleHeader() const
	{
		return mTree.Get<TriangleHeader>(HeaderSize);
	}

	/// Get root of resulting tree
	inline const void *				GetRoot() const
	{
		return mTree.Get<void>(HeaderSize + TriangleHeaderSize);
	}
	
private:
	ByteBuffer						mTree;									///< Resulting tree structure
	uint							mNodesSize;								///< Size in bytes of the nodes in the buffer
};

JPH_NAMESPACE_END
