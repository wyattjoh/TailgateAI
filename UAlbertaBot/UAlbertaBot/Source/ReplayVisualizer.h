#pragma once

#include "..\..\AdversarialSearch\source\common.h"

#ifdef USING_VISUALIZATION_LIBRARIES

#include <Common.h>

#include "InformationManager.h"
#include "MapGrid.h"

#include "..\..\AdversarialSearch\source\Display.h"
#include "..\..\AdversarialSearch\source\GameState.h"
#include "..\..\AdversarialSearch\source\Game.h"
#include "..\..\AdversarialSearch\source\Map.hpp"
#include "..\..\AdversarialSearch\source\Player.h"

typedef boost::shared_ptr<MicroSearch::Player> PlayerPtr;

class ReplayVisualizer 
{
	MicroSearch::Map map;

	const IDType getPlayer(BWAPI::Unit * unit) const;
	const IDType getPlayer(BWAPI::Player * player) const;
	void setCombatUnits(MicroSearch::GameState & s, const BWAPI::Position & center, const int radius);
	const MicroSearch::Unit getUnit(const UnitInfo & ui, const IDType & playerID) const;
	bool isCombatUnit(BWAPI::UnitType type) const;

	PlayerPtr getSearchPlayer(const IDType & playerToMoveMethod, const IDType & playerID, const IDType & evalMethod, const size_t & timeLimitMS);

public:

	ReplayVisualizer();
	void ReplayVisualizer::launchSimulation(const BWAPI::Position & pos, const int & radius);
};

#endif