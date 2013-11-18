#pragma once

#include "BWAPI.h"
#include "BaseTypes.hpp"
#include "assert.h"
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <intrin.h>
#include <sstream>

// #define USING_VISUALIZATION_LIBRARIES

// SEARCH PARAMETERS
namespace Search
{
	// TYPE OF SEARCH TO BE PERFORMED
	namespace SearchMethods
	{
		enum { AlphaBeta, IDAlphaBeta, NegaMax, MiniMax };
	}

	// PLAYERS IN THE GAME
	namespace Players
	{
		enum { Player_One = 0, Player_Two = 1, Player_None = 2, Player_Both = 3};
		extern std::vector<std::string> names;
	}

	namespace PlayerModels
	{
		enum { AlphaBeta, AttackClosest, Kiter, Random, AttackWeakest, AttackDPS, KiterDPS, No_Overkill_DPS, None };
		extern std::vector<std::string> names;
	}

	namespace EvaluationMethods
	{
		enum { SumHP, SumDPS, ModelSimulation, LTD2Bonus };
		extern std::vector<std::string> names;
	}

	namespace IDAThresholdMethod
	{
		enum { Depth, GameTime };
	}

	namespace PlayerToMove
	{
		enum { Random, Alternate, Not_Alternate, ABAB, BABA, ABBA, BAAB };
		extern std::vector<std::string> names;
	}

	// CONSTANTS FOR SEARCH
	namespace Constants
	{
		// NUMBER OF PLAYERS IN THE GAME
		const size_t Num_Players				= 2;
		
		// MAXIMUM NUMBER OF UNITS A PLAYER CAN HAVE
		const size_t Max_Units					= 8;

		// MAX DEPTH THE SEARCH CAN EVER HANDLE
		const size_t Max_Search_Depth			= 10;

		// NUMBER OF DIRECTIONS THAT UNITS CAN MOVE
		const size_t Num_Directions				= 4;

		// MAX NUMBER OF ORDERED MOVES IN A SEARCH DEPTH
		const size_t Max_Ordered_Moves			= 10;

		// DISTANCE MOVED FOR A 'Move' COMMAND
		const size_t Move_Distance				= 16;

		// MAXIMUM NUMBER OF MOVES POSSIBLE FOR ANY UNIT
		const size_t Max_Moves					= Max_Units + Num_Directions + 1;
		const bool   Use_Unit_Bounding			= false;
		const size_t Pass_Move_Duration			= 20;
		const float  Min_Unit_DPF				= 0.1f;
		const HealthType Starting_Energy		= 50;

		// whether to use transposition table in search
		const bool   Use_Transposition_Table	= true;
		const size_t Transposition_Table_Size	= 100000;
		const size_t Transposition_Table_Scan	= 10;
		const size_t Num_Hashes					= 2;

		// RNG seeding options
		const bool Seed_Hash_Time				= false;
		const bool Seed_Player_Random_Time		= true;

		// DIRECTIONS OF MOVEMENT
		const int Move_Dir[4][2] = {{-1,0}, {1,0}, {0,1}, {0,-1} };
	}
	
	namespace StarcraftData
	{
		// vector which will hold manually extracted attack frame data
		extern std::vector<AttackFrameData> attackFrameData;

		void init();
		const AttackFrameData & getAttackFrames(const BWAPI::UnitType & type);
	};
};


// the number of players in the game
#define PRINT_GRAPHVIZ_DATA false

//#define COMPUTE_PV

// stolen from http://stackoverflow.com/questions/1528727/why-is-sse-scalar-sqrtx-slower-than-rsqrtx-x
// stolen from http://stackoverflow.com/questions/1528727/why-is-sse-scalar-sqrtx-slower-than-rsqrtx-x
inline void FAST_SQRT(float * pOut, float * pIn)
{
	__m128 in = _mm_load_ss( pIn );
	_mm_store_ss( pOut, _mm_mul_ss( in, _mm_rsqrt_ss( in ) ) );
   // compiles to movss, movaps, rsqrtss, mulss, movss
}
