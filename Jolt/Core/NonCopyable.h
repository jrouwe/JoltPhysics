// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

/// Class that makes another class non-copyable. Usage: Inherit from NonCopyable.
class NonCopyable
{
public:
	inline	NonCopyable() { }
			NonCopyable(const NonCopyable &) = delete;
	void	operator = (const NonCopyable &) = delete;
};

} // JPH