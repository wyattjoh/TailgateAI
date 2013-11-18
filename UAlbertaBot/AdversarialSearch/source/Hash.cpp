#include "Hash.h"
#include <iostream>

using namespace MicroSearch;

namespace MicroSearch
{
	namespace Hash
	{
		HashType		unitIndexHash[Search::Constants::Num_Players][Search::Constants::Max_Units];
		HashValues		values[Search::Constants::Num_Hashes];
	}
}

const HashType Hash::HashValues::positionHash(const IDType & player, const PositionType & x, const PositionType & y) const
{
	// return hash32shift(unitPositionHash[player] ^ ((x << 16) + y))
	return hash32shift(hash32shift(unitPositionHash[player] ^ x) ^ y);
}

Hash::HashValues::HashValues(int seed)
{
	boost::random::mt19937 gen;
	gen.seed(seed);
	boost::random::uniform_int_distribution<> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	if (Search::Constants::Seed_Hash_Time)
	{
		gen.seed(static_cast<unsigned int>(std::time(0)));
	}

	for (size_t p(0); p<Search::Constants::Num_Players; ++p)
	{
				
		for (size_t u(0); u<Search::Constants::Max_Units; ++u)
		{
			unitIndexHash[p][u] = dist(gen);
		}

		unitPositionHash[p]		= dist(gen);
		timeCanAttackHash[p]	= dist(gen);
		timeCanMoveHash[p]		= dist(gen);
		unitTypeHash[p]			= dist(gen);
		currentHPHash[p]		= dist(gen);
	}
}

const HashType Hash::HashValues::getAttackHash (const size_t & player, const size_t & value) const		
{ 
	return hash32shift(timeCanAttackHash[player] ^ value); 
}


const HashType Hash::HashValues::getMoveHash		(const size_t & player, const size_t & value) const		{ return hash32shift(timeCanMoveHash[player] ^ value); }
const HashType Hash::HashValues::getUnitTypeHash	(const size_t & player, const size_t & value) const		{ return hash32shift(unitTypeHash[player] ^ value); }
const HashType Hash::HashValues::getCurrentHPHash	(const size_t & player, const size_t & value) const		{ return hash32shift(currentHPHash[player] ^ value); }

//Robert Jenkins' 32 bit integer hash function
const size_t Hash::jenkinsHash( size_t a)
{
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

void Hash::initHash()
{
	values[0] = HashValues(0);
	values[1] = HashValues(1);
}

int Hash::hash32shift(int key)
{
	key = ~key + (key << 15); // key = (key << 15) - key - 1;
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057; // key = (key + (key << 3)) + (key << 11);
	key = key ^ (key >> 16);
	return key;
}

const int Hash::jenkinsHashCombine(const HashType & hash, const int val)
{
	return hash32shift(hash ^ (HashType)val);
}

const size_t Hash::magicHash(const HashType & hash, const size_t & player, const size_t & index)
{
	return hash32shift(hash ^ unitIndexHash[player][index]);
}
