// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Core/LinearCurve.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(LinearCurve::Point)
{
	JPH_ADD_ATTRIBUTE(Point, mX)
	JPH_ADD_ATTRIBUTE(Point, mY)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(LinearCurve)
{
	JPH_ADD_ATTRIBUTE(LinearCurve, mPoints)
}

float LinearCurve::GetValue(float inX) const
{
	if (mPoints.empty())
		return 0.0f;

	Points::const_iterator i2 = lower_bound(mPoints.begin(), mPoints.end(), inX, [](const Point &inPoint, float inValue) { return inPoint.mX < inValue; });

	if (i2 == mPoints.begin())
		return mPoints.front().mY;
	else if (i2 == mPoints.end())
		return mPoints.back().mY;

	Points::const_iterator i1 = i2 - 1;
	return i1->mY + (inX - i1->mX) * (i2->mY - i1->mY) / (i2->mX - i1->mX);
}

void LinearCurve::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mPoints);
}

void LinearCurve::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mPoints);
}

} // JPH