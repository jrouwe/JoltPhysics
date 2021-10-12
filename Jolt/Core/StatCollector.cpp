// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Core/StatCollector.h>
#include <Core/Color.h>
#include <Core/StringTools.h>
#include <fstream>

#ifdef JPH_STAT_COLLECTOR

namespace JPH {

StatCollector StatCollector::sInstance;

string StatCollector::Variant::ToString() const
{
	switch (mType)
	{
	case EType::Float:
		return ConvertToString(mFloat);
	case EType::Int:
		return ConvertToString(mInt);
	case EType::Bool:
		return ConvertToString(mBool);
	case EType::Undefined:
	default:
		JPH_ASSERT(false);
		return "";
	}
}

void StatCollector::SetNextFrame()
{
	lock_guard lock(mMutex);

	if (mIsCapturing)
		mCurrentFrame = &mFrames[mCurrentFrameNumber++];
}

void StatCollector::ResetInternal()
{
	mCurrentFrameNumber = 0;
	mCurrentFrame = nullptr;

	mFrames.clear();
	mKeys.clear();
	mNextKey = 0;
}

void StatCollector::Reset()
{
	lock_guard lock(mMutex);

	ResetInternal();	
}

void StatCollector::StartCapture()
{
	lock_guard lock(mMutex);

	ResetInternal();

	mIsCapturing = true;
}

void StatCollector::AddItem(const string &inName, const Variant &inValue)
{
	lock_guard lock(mMutex);

	JPH_ASSERT(mCurrentFrame != nullptr, "Don't forget to call SetFrame(...)");

	// Determine key for inName
	pair<KeyIDMap::iterator, bool> p = mKeys.insert(KeyIDMap::value_type(inName, mNextKey));

	// Increment key inName was new
	if (p.second)
		++mNextKey;

	// Take key from map
	int key = p.first->second;

	// Store value
	(*mCurrentFrame)[key] = inValue;
}

void StatCollector::AddItem(const string &inName, Vec3Arg inValue)
{
	AddItem(inName + ".X", inValue.GetX());
	AddItem(inName + ".Y", inValue.GetY());
	AddItem(inName + ".Z", inValue.GetZ());
}

void StatCollector::AddItem(const string &inName, QuatArg inValue)
{
	Vec3 axis;
	float angle;
	inValue.GetAxisAngle(axis, angle);
	AddItem(inName + ".Axis", axis);
	AddItem(inName + ".Angle", RadiansToDegrees(angle));
}

struct StatTreeNode
{
	using Children = map<string, StatTreeNode>;

	int			mIndex = -1;
	Children	mChildren;
};

static void sWriteStatTree(ofstream &ioStream, const StatTreeNode &inNode)
{
	bool first = true;
	for (const StatTreeNode::Children::value_type &c : inNode.mChildren)
	{
		// Write comma if this is not the first line
		if (!first)
			ioStream << ",";
		first = false;

		// Write title
		ioStream << "{title:\"" + c.first + "\"";
		
		// Write key
		ioStream << ",key:\"" + ConvertToString(c.second.mIndex) + "\"";
		
		// Write children
		if (!c.second.mChildren.empty())
		{
			ioStream << ",children:[";
				sWriteStatTree(ioStream, c.second);
			ioStream << "]";
		}

		ioStream << "}";
	}
}

void StatCollector::StopCapture(const char *inFileName)
{
	lock_guard lock(mMutex);

	// Stop capturing
	mIsCapturing = false;

	// Open file
	ofstream f;
	f.open(inFileName, ofstream::out | ofstream::trunc);
	if (!f.is_open()) 
		return;

	// Start html file
	f << R"(<!DOCTYPE html>
<html>
	<head>
		<title>Stats</title>
		<script type="text/javascript" src="WebIncludes/jquery-3.2.1.min.js"></script>
		<script src="WebIncludes/dygraph.min.js"></script>
		<link rel="stylesheet" href="WebIncludes/dygraph.min.css"/>
		<script src="WebIncludes/jquery.fancytree-all-deps.min.js"></script>
		<link rel="stylesheet" href="WebIncludes/ui.fancytree.min.css"/>
		<style>
			#labelsdiv>span { display: block; }
			ul.fancytree-container { border: 0px; }
		</style>	
	</head>
	<body>
	<div style="width: 100%; height: 50vh;">
		<div id="graphdiv" style="float: left; width:60%; height: 50vh; overflow: hidden;"></div>
		<div id="labelsdiv" style="float: right; width:39%; height: 50vh; overflow-x: hidden; overflow-y: scroll;"></div>
	</div>
	<p>
		<button id="btnSelectAll">Select All</button> &nbsp; 
		<button id="btnDeselectAll">Deselect All</button> &nbsp; 
		<input id="search" placeholder="Filter..." autocomplete="off">
		<button id="btnResetSearch">&times;</button>
		<span id="matches"></span>
	</p>
	<div style="width:100%; height: 40vh; overflow-x: hidden; overflow-y: scroll;">
		<div id="tree" style="width:100%;">
		</div>
	</div>
	<script type="text/javascript">
		"use strict";
)";

	// Write all data points
	f << "var point_data = [";
	bool first = true;
	for (const FrameMap::value_type &entry : mFrames)
	{
		// Don't write empty samples
		if (entry.second.empty())
			continue;

		// Write comma at start of each next line
		if (!first)
			f << ",";
		first = false;

		// Write frame number
		f << "[" << entry.first;

		// Write all columns
		for (const KeyIDMap::value_type &key : mKeys)
		{
			KeyValueMap::const_iterator v = entry.second.find(key.second);
			if (v == entry.second.end())
				f << ",NaN";
			else
				f << "," << v->second.ToString();
		}

		f << "]\n";
	}
	f << "];\n";

	// Write labels
	f << "var labels_data = [\"Frame\"";
	for (const KeyIDMap::value_type &key : mKeys)
		f << ",\"" << key.first << "\"";
	f << "];\n";

	// Write colors
	f << "var colors_data = ['rgb(0,0,0)'";
	for (int i = 0; i < (int)mKeys.size() - 1; ++i)
	{
		Color c = Color::sGetDistinctColor(i);
		f << ",'rgb(" << (int)c.r << "," << (int)c.g << "," << (int)c.b << ")'";
	}
	f << "];\n";

	// Calculate tree
	StatTreeNode root;
	int index = 0;
	for (const KeyIDMap::value_type &key : mKeys)
	{
		// Split parts of key
		vector<string> parts;
		StringToVector(key.first, parts, ".");

		// Create tree nodes
		StatTreeNode *cur = &root;
		for (string p : parts)
			cur = &cur->mChildren[p];

		// Set index on leaf node
		cur->mIndex = index;
		++index;
	}

	// Output tree
	f << "var tree_data = [";
	sWriteStatTree(f, root);
	f << "];\n";

	// Write main script
	f << R"-(
		var graph = new Dygraph(
			document.getElementById("graphdiv"),
			point_data,
			{ 
			labels: labels_data,
			colors: colors_data,
			labelsDiv: labelsdiv,
			hideOverlayOnMouseOut: false,
			showRangeSelector: true,
			xlabel: "Frame",
			ylabel: "Value"
		});

		function sync_enabled_series(tree, graph) {
			var is_visible = [];
			for (var i = 0; i < graph.numColumns() - 1; ++i)
				is_visible[i] = false;

			var selected_nodes = tree.getSelectedNodes(false);
			for (var i = 0; i < selected_nodes.length; ++i)
			{
				var key = parseInt(selected_nodes[i].key);
				if (key >= 0)
					is_visible[key] = true;
			}
				
			graph.setVisibility(is_visible);
		};

		$(function() {
			$("#tree").fancytree({
				extensions: ["filter"],
				quicksearch: true,
				source: tree_data,
				icon: false,
				checkbox: true,
				selectMode: 3,
				keyboard: true,
				quicksearch: true,
				filter: {
					autoExpand: true,			
					mode: "hide"
				},
				select: function(event, data) {
					sync_enabled_series(tree, graph);
				}
			});
		
			var tree = $("#tree").fancytree("getTree");
		
			var no_events = { noEvents: true };

			tree.enableUpdate(false);
			tree.visit(function(node) {
				node.setExpanded(true);
				node.setSelected(true, no_events);
			});	
			tree.enableUpdate(true);
		
			$("#search").keyup(function(e) {
				var match = $(this).val();
				if (e && e.which === $.ui.keyCode.ESCAPE || $.trim(match) === "") {
					$("#btnResetSearch").click();
					return;
				}
				var n = tree.filterBranches.call(tree, match, { autoExpand: true });
				$("#btnResetSearch").attr("disabled", false);
				$("#matches").text("(" + n + " matches)");
			}).focus();

			$("#btnResetSearch").click(function(e) {
				$("#search").val("");
				$("#btnResetSearch").attr("disabled", true);
				$("#matches").text("");
				tree.clearFilter();
			}).attr("disabled", true);

			$("#btnDeselectAll").click(function() {
				tree.enableUpdate(false);
				tree.visit(function(node) {
					if (node.isMatched())
						node.setSelected(false, no_events);
					});
				tree.enableUpdate(true);
				sync_enabled_series(tree, graph);
				return false;
			});
		
			$("#btnSelectAll").click(function() {
				tree.enableUpdate(false);
				tree.visit(function(node) {
					if (node.isMatched())
						node.setSelected(true, no_events);
					});
				tree.enableUpdate(true);
				sync_enabled_series(tree, graph);
				return false;
			});
		});
	</script>
	</body>
</html>)-";

	// Remove all collected data
	ResetInternal();
}

} // JPH

#endif // JPH_STAT_COLLECTOR
