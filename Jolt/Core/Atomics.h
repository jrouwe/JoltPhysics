// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>

namespace JPH {

/// Atomically compute the min(ioAtomic, inValue) and store it in ioAtomic, returns true if value was updated
template <class T>
bool AtomicMin(atomic<T> &ioAtomic, const T inValue)
{
	T cur_value = ioAtomic;
	while (cur_value > inValue)
		if (ioAtomic.compare_exchange_weak(cur_value, inValue))
			return true;
	return false;
}

/// Atomically compute the max(ioAtomic, inValue) and store it in ioAtomic, returns true if value was updated
template <class T>
bool AtomicMax(atomic<T> &ioAtomic, const T inValue)
{
	T cur_value = ioAtomic;
	while (cur_value < inValue)
		if (ioAtomic.compare_exchange_weak(cur_value, inValue))
			return true;
	return false;
}

} // JPH