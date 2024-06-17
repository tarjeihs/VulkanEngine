#include "EnginePCH.h"
#include "UUID.h"
#include <random>

namespace Utils
{
	static std::random_device RandomDevice;
	static std::mt19937_64 Engine(RandomDevice());
	static std::uniform_int_distribution<uint64_t> UniformDistribution;
}

SUUID::SUUID()
	: UUID(Utils::UniformDistribution(Utils::Engine))
{
}

SUUID::SUUID(uint64_t InUUID)
	: UUID(InUUID)
{
}
