#pragma once

#include <cstddef>

#include "Math/MathTypes.h"

struct SUUID 
{
	SUUID();
	SUUID(uint64 InUUID);
	SUUID(const SUUID&) = default;

	operator uint64() const
	{
		return UUID;
	}
private:
	uint64 UUID;
};

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<SUUID>
	{
		std::size_t operator()(const SUUID& UUID) const noexcept
		{
			return (uint64)UUID;
		}
	};
};