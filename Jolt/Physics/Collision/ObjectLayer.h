// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

/// Layer that objects can be in, determines which other objects it can collide with
using ObjectLayer = uint16;

/// Constant value used to indicate an invalid object layer
static constexpr ObjectLayer cObjectLayerInvalid = 0xffff;

/// Filter class for object layers
class ObjectLayerFilter : public NonCopyable
{
public:
	/// Destructor
	virtual					~ObjectLayerFilter() = default;

	/// Function to filter out object layers when doing collision query test (return true to allow testing against objects with this layer)
	virtual bool			ShouldCollide(ObjectLayer inLayer) const
	{
		return true;
	}

#ifdef JPH_TRACK_BROADPHASE_STATS
	/// Get a string that describes this filter for stat tracking purposes
	virtual string			GetDescription() const
	{
		return "No Description";
	}
#endif // JPH_TRACK_BROADPHASE_STATS
};

/// Function to test if two objects can collide based on their object layer. Used while finding collision pairs.
using ObjectLayerPairFilter = bool (*)(ObjectLayer inLayer1, ObjectLayer inLayer2);

/// Default filter class that uses the pair filter in combination with a specified layer to filter layers
class DefaultObjectLayerFilter : public ObjectLayerFilter
{
public:
	/// Constructor
							DefaultObjectLayerFilter(ObjectLayerPairFilter inObjectLayerPairFilter, ObjectLayer inLayer) :
		mObjectLayerPairFilter(inObjectLayerPairFilter),
		mLayer(inLayer)
	{
	}

	/// Copy constructor
							DefaultObjectLayerFilter(const DefaultObjectLayerFilter &inRHS) :
		mObjectLayerPairFilter(inRHS.mObjectLayerPairFilter),
		mLayer(inRHS.mLayer)
	{
	}

	// See ObjectLayerFilter::ShouldCollide
	virtual bool			ShouldCollide(ObjectLayer inLayer) const override
	{
		return mObjectLayerPairFilter(mLayer, inLayer);
	}

private:
	ObjectLayerPairFilter	mObjectLayerPairFilter;
	ObjectLayer				mLayer;
};

/// Allows objects from a specific layer only
class SpecifiedObjectLayerFilter : public ObjectLayerFilter
{
public:
	/// Constructor
	explicit				SpecifiedObjectLayerFilter(ObjectLayer inLayer) :
		mLayer(inLayer)
	{
	}

	// See ObjectLayerFilter::ShouldCollide
	virtual bool			ShouldCollide(ObjectLayer inLayer) const override
	{
		return mLayer == inLayer;
	}

private:
	ObjectLayer				mLayer;
};

JPH_NAMESPACE_END
