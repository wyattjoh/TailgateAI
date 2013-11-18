#pragma once

#include "Common.h"

// SEARCH PARAMETERS
namespace Search
{
	namespace Players				{ std::vector<std::string> names; }
	namespace PlayerModels			{ std::vector<std::string> names; }
	namespace EvaluationMethods		{ std::vector<std::string> names; }
	namespace PlayerToMove			{ std::vector<std::string> names; }

	namespace StarcraftData
	{
		// vector which will hold manually extracted attack frame data
		std::vector<AttackFrameData> attackFrameData;

		void init()
		{
			// initialize BWAPI
			BWAPI::BWAPI_init();

			// allocate the vector according to UnitType size
			attackFrameData = std::vector<AttackFrameData>(BWAPI::UnitTypes::allUnitTypes().size(), AttackFrameData(0,0));
	
			// Protoss Units
			attackFrameData[BWAPI::UnitTypes::Protoss_Probe.getID()]				= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Protoss_Zealot.getID()]				= AttackFrameData(8, 7);
			attackFrameData[BWAPI::UnitTypes::Protoss_Dragoon.getID()]				= AttackFrameData(7, 3);
			attackFrameData[BWAPI::UnitTypes::Protoss_Dark_Templar.getID()]			= AttackFrameData(9, 9);
			attackFrameData[BWAPI::UnitTypes::Protoss_Scout.getID()]				= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Protoss_Corsair.getID()]				= AttackFrameData(8, 8);
			attackFrameData[BWAPI::UnitTypes::Protoss_Arbiter.getID()]				= AttackFrameData(2, 2);

			// Terran Units
			attackFrameData[BWAPI::UnitTypes::Terran_SCV.getID()]					= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Terran_Marine.getID()]				= AttackFrameData(8, 6);
			attackFrameData[BWAPI::UnitTypes::Terran_Firebat.getID()]				= AttackFrameData(8, 8);
			attackFrameData[BWAPI::UnitTypes::Terran_Ghost.getID()]					= AttackFrameData(3, 2);
			attackFrameData[BWAPI::UnitTypes::Terran_Vulture.getID()]				= AttackFrameData(1, 1);
			attackFrameData[BWAPI::UnitTypes::Terran_Goliath.getID()]				= AttackFrameData(1, 1);
			attackFrameData[BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode.getID()]	= AttackFrameData(1, 1);
			attackFrameData[BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode.getID()]	= AttackFrameData(1, 1);
			attackFrameData[BWAPI::UnitTypes::Terran_Wraith.getID()]				= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Terran_Battlecruiser.getID()]			= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Terran_Valkyrie.getID()]				= AttackFrameData(40, 40);

			// Zerg Units
			attackFrameData[BWAPI::UnitTypes::Zerg_Drone.getID()]					= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Zerg_Zergling.getID()]				= AttackFrameData(5, 5);
			attackFrameData[BWAPI::UnitTypes::Zerg_Hydralisk.getID()]				= AttackFrameData(3, 2);
			attackFrameData[BWAPI::UnitTypes::Zerg_Lurker.getID()]					= AttackFrameData(2, 2);
			attackFrameData[BWAPI::UnitTypes::Zerg_Ultralisk.getID()]				= AttackFrameData(14, 14);
			attackFrameData[BWAPI::UnitTypes::Zerg_Mutalisk.getID()]				= AttackFrameData(1, 1);
			attackFrameData[BWAPI::UnitTypes::Zerg_Devourer.getID()]				= AttackFrameData(9, 9);

			// Initialize enum namings
			// enum { Player_One = 0, Player_Two = 1, Player_None = 2, Player_Both = 3};
			Search::Players::names.push_back("Player One");
			Search::Players::names.push_back("Player Two");
			Search::Players::names.push_back("Player None");
			Search::Players::names.push_back("Player Both");

			// enum { AlphaBeta, AttackClosest, Kiter, Random, AttackWeakest, AttackDPS, KiterDPS, No_Overkill_DPS, None };
			Search::PlayerModels::names.push_back("AlphaBeta");
			Search::PlayerModels::names.push_back("AttackClosest");
			Search::PlayerModels::names.push_back("Kiter");
			Search::PlayerModels::names.push_back("Random");
			Search::PlayerModels::names.push_back("AttackWeakest");
			Search::PlayerModels::names.push_back("AttackDPS");
			Search::PlayerModels::names.push_back("KiterDPS");
			Search::PlayerModels::names.push_back("NOKDPS");
			Search::PlayerModels::names.push_back("None");

			// enum { SumHP, SumDPS, ModelSimulation };
			Search::EvaluationMethods::names.push_back("LTD");
			Search::EvaluationMethods::names.push_back("LTD2");
			Search::EvaluationMethods::names.push_back("Playout");
			Search::EvaluationMethods::names.push_back("LTD2Bonus");

			//enum { Random, Alternate, Not_Alternate, ABAB, BABA, ABBA, BAAB };
			Search::PlayerToMove::names.push_back("Random");
			Search::PlayerToMove::names.push_back("Alternate");
			Search::PlayerToMove::names.push_back("Not Alternate");
			Search::PlayerToMove::names.push_back("ABAB");
			Search::PlayerToMove::names.push_back("BABA");
			Search::PlayerToMove::names.push_back("ABBA");
			Search::PlayerToMove::names.push_back("BAAB");
		}
		const AttackFrameData & getAttackFrames(const BWAPI::UnitType & type)
		{
			return attackFrameData[type.getID()];
		}
	};
};

// the number of players in the game
#define PRINT_GRAPHVIZ_DATA false

//#define COMPUTE_PV


