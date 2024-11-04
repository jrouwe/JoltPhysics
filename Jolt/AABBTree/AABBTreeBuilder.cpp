// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/AABBTree/AABBTreeBuilder.h>

JPH_NAMESPACE_BEGIN

AABBTreeBuilder::Node::Node()
{
	mChildIndices[0] = invalidNodeIndex();
	mChildIndices[1] = invalidNodeIndex();
	num_tris = 0;
}

AABBTreeBuilder::Node::~Node()
{
}

uint AABBTreeBuilder::Node::GetMinDepth(const Array<Node>& nodes) const
{
	if (HasChildren())
	{
		uint left = nodes[mChildIndices[0]].GetMinDepth(nodes);
		uint right = nodes[mChildIndices[1]].GetMinDepth(nodes);
		return min(left, right) + 1;
	}
	else
		return 1;
}

uint AABBTreeBuilder::Node::GetMaxDepth(const Array<Node>& nodes) const
{
	if (HasChildren())
	{
		uint left = nodes[mChildIndices[0]].GetMaxDepth(nodes);
		uint right = nodes[mChildIndices[1]].GetMaxDepth(nodes);
		return max(left, right) + 1;
	}
	else
		return 1;
}

uint AABBTreeBuilder::Node::GetNodeCount(const Array<Node>& nodes) const
{
	if (HasChildren())
		return nodes[mChildIndices[0]].GetNodeCount(nodes) + nodes[mChildIndices[1]].GetNodeCount(nodes) + 1;
	else
		return 1;
}

uint AABBTreeBuilder::Node::GetLeafNodeCount(const Array<Node>& nodes) const
{
	if (HasChildren())
		return nodes[mChildIndices[0]].GetLeafNodeCount(nodes) + nodes[mChildIndices[1]].GetLeafNodeCount(nodes);
	else
		return 1;
}

uint AABBTreeBuilder::Node::GetTriangleCountInTree(const Array<Node>& nodes) const
{
	if (HasChildren())
		return nodes[mChildIndices[0]].GetTriangleCountInTree(nodes) + nodes[mChildIndices[1]].GetTriangleCountInTree(nodes);
	else
		return GetTriangleCount();
}

void AABBTreeBuilder::Node::GetTriangleCountPerNode(const Array<Node>& nodes, float &outAverage, uint &outMin, uint &outMax) const
{
	outMin = INT_MAX;
	outMax = 0;
	outAverage = 0;
	uint avg_divisor = 0;
	GetTriangleCountPerNodeInternal(nodes, outAverage, avg_divisor, outMin, outMax);
	if (avg_divisor > 0)
		outAverage /= avg_divisor;
}

float AABBTreeBuilder::Node::CalculateSAHCost(const Array<Node>& nodes, float inCostTraversal, float inCostLeaf) const
{
	float surface_area = mBounds.GetSurfaceArea();
	return surface_area > 0.0f? CalculateSAHCostInternal(nodes, inCostTraversal / surface_area, inCostLeaf / surface_area) : 0.0f;
}

void AABBTreeBuilder::Node::GetNChildren(const Array<Node>& nodes, uint inN, Array<const Node*> &outChildren) const
{
	JPH_ASSERT(outChildren.empty());

	// Check if there is anything to expand
	if (!HasChildren())
		return;

	// Start with the children of this node
	outChildren.push_back(&nodes[mChildIndices[0]]);
	outChildren.push_back(&nodes[mChildIndices[1]]);

	size_t next = 0;
	bool all_triangles = true;
	while (outChildren.size() < inN)
	{
		// If we have looped over all nodes, start over with the first node again
		if (next >= outChildren.size())
		{
			// If there only triangle nodes left, we have to terminate
			if (all_triangles)
				return;
			next = 0;
			all_triangles = true;
		}

		// Try to expand this node into its two children
		const Node *to_expand = outChildren[next];
		if (to_expand->HasChildren())
		{
			outChildren.erase(outChildren.begin() + next);
			outChildren.push_back(&nodes[to_expand->mChildIndices[0]]);
			outChildren.push_back(&nodes[to_expand->mChildIndices[1]]);
			all_triangles = false;
		}
		else
		{
			++next;
		}
	}
}

float AABBTreeBuilder::Node::CalculateSAHCostInternal(const Array<Node>& nodes, float inCostTraversalDivSurfaceArea, float inCostLeafDivSurfaceArea) const
{
	if (HasChildren())
		return inCostTraversalDivSurfaceArea * mBounds.GetSurfaceArea()
			+ nodes[mChildIndices[0]].CalculateSAHCostInternal(nodes, inCostTraversalDivSurfaceArea, inCostLeafDivSurfaceArea)
			+ nodes[mChildIndices[1]].CalculateSAHCostInternal(nodes, inCostTraversalDivSurfaceArea, inCostLeafDivSurfaceArea);
	else
		return inCostLeafDivSurfaceArea * mBounds.GetSurfaceArea() * GetTriangleCount();
}

void AABBTreeBuilder::Node::GetTriangleCountPerNodeInternal(const Array<Node>& nodes, float &outAverage, uint &outAverageDivisor, uint &outMin, uint &outMax) const
{
	if (HasChildren())
	{
		nodes[mChildIndices[0]].GetTriangleCountPerNodeInternal(nodes, outAverage, outAverageDivisor, outMin, outMax);
		nodes[mChildIndices[1]].GetTriangleCountPerNodeInternal(nodes, outAverage, outAverageDivisor, outMin, outMax);
	}
	else
	{
		outAverage += GetTriangleCount();
		outAverageDivisor++;
		outMin = min(outMin, GetTriangleCount());
		outMax = max(outMax, GetTriangleCount());
	}
}

AABBTreeBuilder::AABBTreeBuilder(TriangleSplitter &inSplitter, uint inMaxTrianglesPerLeaf) :
	mTriangleSplitter(inSplitter),
	mMaxTrianglesPerLeaf(inMaxTrianglesPerLeaf)
{
}

uint AABBTreeBuilder::Build(AABBTreeBuilderStats &outStats)
{
	TriangleSplitter::Range initial = mTriangleSplitter.GetInitialRange();

	// Worst case for number of nodes: 1 leaf node per triangle, so num leaf nodes = num tris
	// For N leaf nodes there are max N - 1 internal nodes.  So <= 2N nodes overall where N = num tris.
	// TODO: test on a few different meshes, use a conservative emperical bound.
	mNodes.reserve(2 * initial.Count());

	mLeafTriangles.reserve(initial.Count());

	const uint root_node_index = BuildInternal(initial);

	Node *root = &mNodes[root_node_index];

	float avg_triangles_per_leaf;
	uint min_triangles_per_leaf, max_triangles_per_leaf;
	root->GetTriangleCountPerNode(mNodes, avg_triangles_per_leaf, min_triangles_per_leaf, max_triangles_per_leaf);

	mTriangleSplitter.GetStats(outStats.mSplitterStats);

	outStats.mSAHCost = root->CalculateSAHCost(mNodes, 1.0f, 1.0f);
	outStats.mMinDepth = root->GetMinDepth(mNodes);
	outStats.mMaxDepth = root->GetMaxDepth(mNodes);
	outStats.mNodeCount = root->GetNodeCount(mNodes);
	outStats.mLeafNodeCount = root->GetLeafNodeCount(mNodes);
	outStats.mMaxTrianglesPerLeaf = mMaxTrianglesPerLeaf;
	outStats.mTreeMinTrianglesPerLeaf = min_triangles_per_leaf;
	outStats.mTreeMaxTrianglesPerLeaf = max_triangles_per_leaf;
	outStats.mTreeAvgTrianglesPerLeaf = avg_triangles_per_leaf;

	return root_node_index;
}

uint AABBTreeBuilder::BuildInternal(const TriangleSplitter::Range &inTriangles)
{
	// Check if there are too many triangles left
	if (inTriangles.Count() > mMaxTrianglesPerLeaf)
	{
		// Split triangles in two batches
		TriangleSplitter::Range left, right;
		if (!mTriangleSplitter.Split(inTriangles, left, right))
		{
			// When the trace below triggers:
			//
			// This code builds a tree structure to accelerate collision detection.
			// At top level it will start with all triangles in a mesh and then divides the triangles into two batches.
			// This process repeats until until the batch size is smaller than mMaxTrianglePerLeaf.
			//
			// It uses a TriangleSplitter to find a good split. When this warning triggers, the splitter was not able
			// to create a reasonable split for the triangles. This usually happens when the triangles in a batch are
			// intersecting. They could also be overlapping when projected on the 3 coordinate axis.
			//
			// To solve this issue, you could try to pass your mesh through a mesh cleaning / optimization algorithm.
			// You could also inspect the triangles that cause this issue and see if that part of the mesh can be fixed manually.
			//
			// When you do not fix this warning, the tree will be less efficient for collision detection, but it will still work.
			JPH_IF_DEBUG(Trace("AABBTreeBuilder: Doing random split for %d triangles (max per node: %u)!", (int)inTriangles.Count(), mMaxTrianglesPerLeaf);)
			int half = inTriangles.Count() / 2;
			JPH_ASSERT(half > 0);
			left = TriangleSplitter::Range(inTriangles.mBegin, inTriangles.mBegin + half);
			right = TriangleSplitter::Range(inTriangles.mBegin + half, inTriangles.mEnd);
		}

		// Recursively build
		const uint node_index = (uint)mNodes.size();
		mNodes.push_back(Node());
		mNodes[node_index].mChildIndices[0] = BuildInternal(left);
		mNodes[node_index].mChildIndices[1] = BuildInternal(right);
		mNodes[node_index].mBounds = mNodes[mNodes[node_index].mChildIndices[0]].mBounds;
		mNodes[node_index].mBounds.Encapsulate(mNodes[mNodes[node_index].mChildIndices[1]].mBounds);
		return node_index;
	}

	// Create leaf node
	const uint node_index = (uint)mNodes.size();
	mNodes.push_back(Node());
	mNodes[node_index].tris_begin = (uint)mLeafTriangles.size();
	mNodes[node_index].num_tris = inTriangles.mEnd - inTriangles.mBegin;
	for (uint i = inTriangles.mBegin; i < inTriangles.mEnd; ++i)
	{
		const IndexedTriangle &t = mTriangleSplitter.GetTriangle(i);
		const VertexList &v = mTriangleSplitter.GetVertices();
		mLeafTriangles.push_back(t);

		mNodes[node_index].mBounds.Encapsulate(v, t);
	}

	return node_index;
}

JPH_NAMESPACE_END
