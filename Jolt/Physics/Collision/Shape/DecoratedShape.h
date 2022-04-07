// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/Shape.h>

JPH_NAMESPACE_BEGIN

/// Class that constructs a DecoratedShape
class DecoratedShapeSettings : public ShapeSettings
{
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(DecoratedShapeSettings)

	/// Default constructor for deserialization
									DecoratedShapeSettings() = default;

	/// Constructor that decorates another shape
	explicit						DecoratedShapeSettings(const ShapeSettings *inShape)	: mInnerShape(inShape) { }
	explicit						DecoratedShapeSettings(const Shape *inShape)			: mInnerShapePtr(inShape) { }

	RefConst<ShapeSettings>			mInnerShape;											///< Sub shape (either this or mShapePtr needs to be filled up)
	RefConst<Shape>					mInnerShapePtr;											///< Sub shape (either this or mShape needs to be filled up)
};

/// Base class for shapes that decorate another shape with extra functionality (e.g. scale, translation etc.)
class DecoratedShape : public Shape
{
public:
	/// Constructor
	explicit						DecoratedShape(EShapeSubType inSubType) : Shape(EShapeType::Decorated, inSubType) { }
									DecoratedShape(EShapeSubType inSubType, const Shape *inInnerShape) : Shape(EShapeType::Decorated, inSubType), mInnerShape(inInnerShape) { }
									DecoratedShape(EShapeSubType inSubType, const DecoratedShapeSettings &inSettings, ShapeResult &outResult);

	/// Access to the decorated inner shape
	const Shape *					GetInnerShape() const									{ return mInnerShape; }

	// See Shape::MustBeStatic
	virtual bool					MustBeStatic() const override							{ return mInnerShape->MustBeStatic(); }

	// See Shape::GetCenterOfMass
	virtual Vec3					GetCenterOfMass() const override						{ return mInnerShape->GetCenterOfMass(); }

	// See Shape::GetSubShapeIDBitsRecursive
	virtual uint					GetSubShapeIDBitsRecursive() const override				{ return mInnerShape->GetSubShapeIDBitsRecursive(); }

	// See Shape::GetMaterial
	virtual const PhysicsMaterial *	GetMaterial(const SubShapeID &inSubShapeID) const override;

	// See Shape::GetSubShapeUserData
	virtual uint64					GetSubShapeUserData(const SubShapeID &inSubShapeID) const override;

	// See Shape
	virtual void					SaveSubShapeState(ShapeList &outSubShapes) const override;
	virtual void					RestoreSubShapeState(const ShapeRefC *inSubShapes, uint inNumShapes) override;

	// See Shape::GetStatsRecursive
	virtual Stats					GetStatsRecursive(VisitedShapes &ioVisitedShapes) const override;

protected:
	RefConst<Shape>					mInnerShape;
};

JPH_NAMESPACE_END
