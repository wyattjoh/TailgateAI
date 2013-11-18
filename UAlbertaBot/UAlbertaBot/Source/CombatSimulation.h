#pragma once

#include "Common.h"
#include "MapGrid.h"

#ifdef USING_VISUALIZATION_LIBRARIES
	#include "Visualizer.h"
#endif

#include "..\..\AdversarialSearch\source\GameState.h"
#include "..\..\AdversarialSearch\source\Game.h"

class CombatSimulation
{

	MicroSearch::GameState		state;
	bool						hasLogged;

public:

	CombatSimulation();

	void setCombatUnits(const BWAPI::Position & center, const int radius);

	std::pair<ScoreType, ScoreType> simulateCombat();
	const MicroSearch::Unit			getUnit(const UnitInfo & ui, const IDType & playerID) const;
	const MicroSearch::GameState &	getState() const;

	const IDType getPlayer(BWAPI::Unit * unit) const;
	const IDType getPlayer(BWAPI::Player * player) const;

	void logState(const MicroSearch::GameState & state);
};