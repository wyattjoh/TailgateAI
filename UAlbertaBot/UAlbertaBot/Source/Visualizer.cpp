#include "Visualizer.h"

#ifdef USING_VISUALIZATION_LIBRARIES

Visualizer & Visualizer::Instance() 
{
	static Visualizer instance;
	return instance;
}

Visualizer::Visualizer() 
	: display(MicroSearch::Display(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight()))
{
	map = MicroSearch::Map(BWAPI::Broodwar);
	map.write("C:\\test.txt");
	display.OnStart();
	
	display.LoadMapTexture(&map, 19);
}

void Visualizer::setBWAPIState()
{
	MicroSearch::GameState state;
	state.setMaxUnits(200);
	state.setMap(&map);

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		const IDType player(getPlayer(unit->getPlayer()));

		if (player == Search::Players::Player_One || player == Search::Players::Player_Two)
		{
			state.addUnit(MicroSearch::Unit(unit, player, BWAPI::Broodwar->getFrameCount()));
		}
		else
		{
			if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field ||
				unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
			{
				state.addNeutralUnit(MicroSearch::Unit(unit, Search::Players::Player_None, BWAPI::Broodwar->getFrameCount()));
			}
		}
	}

	setState(state);
}

void Visualizer::setState(const MicroSearch::GameState & state)
{
	display.SetState(state);
}

void Visualizer::onFrame()
{
	display.OnFrame();
}

const IDType Visualizer::getPlayer(BWAPI::Unit * unit) const
{
	return getPlayer(unit->getPlayer());
}

const IDType Visualizer::getPlayer(BWAPI::Player * player) const
{
	if (player == BWAPI::Broodwar->self())
	{
		return Search::Players::Player_One;
	}
	else if (player == BWAPI::Broodwar->enemy())
	{
		return Search::Players::Player_Two;
	}

	return Search::Players::Player_None;
}

#endif