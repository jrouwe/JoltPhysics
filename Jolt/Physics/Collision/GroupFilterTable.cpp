// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/GroupFilterTable.h>
#include <ObjectStream/TypeDeclarations.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(GroupFilterTable)
{
	JPH_ADD_BASE_CLASS(GroupFilterTable, GroupFilter)

	JPH_ADD_ATTRIBUTE(GroupFilterTable, mNumSubGroups)
	JPH_ADD_ATTRIBUTE(GroupFilterTable, mTable)
}

void GroupFilterTable::SaveBinaryState(StreamOut &inStream) const
{
	GroupFilter::SaveBinaryState(inStream);

	inStream.Write(mNumSubGroups);
	inStream.Write(mTable);
}

void GroupFilterTable::RestoreBinaryState(StreamIn &inStream)
{
	GroupFilter::RestoreBinaryState(inStream);

	inStream.Read(mNumSubGroups);
	inStream.Read(mTable);
}

} // JPH