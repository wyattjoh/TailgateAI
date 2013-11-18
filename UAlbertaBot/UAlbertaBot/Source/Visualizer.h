#pragma once

#include "..\..\AdversarialSearch\source\common.h"

#ifdef USING_VISUALIZATION_LIBRARIES

#include <Common.h>

#include "..\..\AdversarialSearch\source\Display.h"
#include "..\..\AdversarialSearch\source\GameState.h"
#include "..\..\AdversarialSearch\source\Map.hpp"

class Visualizer 
{
	Visualizer();
	Visualizer(int mapWidth, int mapHeight, int cellSize);

	MicroSearch::Display display;
	MicroSearch::GameState state;
	MicroSearch::Map map;

	const IDType getPlayer(BWAPI::Unit * unit) const;
	const IDType getPlayer(BWAPI::Player * player) const;

public:

	// yay for singletons!
	static Visualizer &	Instance();
	
	void setBWAPIState();
	void setState(const MicroSearch::GameState & state);
	void onFrame();
};

#endif