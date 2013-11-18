#pragma once

#include "BWAPI.h"

namespace Options
{
	namespace Modules
	{
		extern const bool USING_GAMECOMMANDER;			// toggle GameCommander, effectively UAlbertaBot
		extern const bool USING_ENHANCED_INTERFACE;		// toggle EnhancedUI, not needed for UAlbertaBot
		extern const bool USING_REPLAY_VISUALIZER;		// toggle replay visualization tool for combat search demo
		extern const bool USING_MICRO_SEARCH;			// toggle use of Micro Search, if false script used
		extern const bool USING_MACRO_SEARCH;			// toggle use of Build Order Search, currently no backup
		extern const bool USING_STRATEGY_IO;			// toggle the use of file io for strategy
	}

	namespace Tournament
	{
		extern const int GAME_END_FRAME;	
	}

	namespace FileIO
	{
		extern const char * FILE_SETTINGS;				
	}

	namespace Debug
	{
		extern bool DRAW_UALBERTABOT_DEBUG;				// draws debug information for UAlbertaBot
		extern bool DRAW_DEBUG_INTERFACE;				// draws debug information for EnhancedUI

		extern BWAPI::Color COLOR_LINE_TARGET;
		extern BWAPI::Color COLOR_LINE_MINERAL;
		extern BWAPI::Color COLOR_UNIT_NEAR_ENEMY;
		extern BWAPI::Color COLOR_UNIT_NOTNEAR_ENEMY;
	}

	namespace Micro
	{
		extern bool WORKER_DEFENSE;
		extern int  WORKER_DEFENSE_PER_UNIT;

		extern int COMBAT_RADIUS;						// radius of combat to consider units for Micro Search
		extern int COMBAT_REGROUP_RADIUS;				// radius of units around frontmost unit we consider in regroup calculation
		extern int UNIT_NEAR_ENEMY_RADIUS;				// radius to consider a unit 'near' to an enemy unit
		extern int MICRO_SEARCH_MAX_TIMER;
		extern int MICRO_SEARCH_MIN_TIMER;
	}

	namespace Tools
	{
		extern int MAP_GRID_SIZE;
	}
}