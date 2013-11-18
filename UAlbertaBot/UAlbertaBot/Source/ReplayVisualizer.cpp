#include "ReplayVisualizer.h"

#ifdef USING_VISUALIZATION_LIBRARIES

#include "Common.h"

ReplayVisualizer::ReplayVisualizer() 
	: map(BWAPI::Broodwar)
{
	
}

void ReplayVisualizer::launchSimulation(const BWAPI::Position & center, const int & radius)
{
	// set up the display object
	MicroSearch::Display display(MicroSearch::Display(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight()));
	display.OnStart();
	display.LoadMapTexture(&map, 19);

	// extract the state from the current state of BWAPI
	MicroSearch::GameState state;
	setCombatUnits(state, center, radius);
	state.setMap(&map);

	// get search player objects for us and the opponent
	PlayerPtr selfPlayer(getSearchPlayer(Search::PlayerToMove::Alternate, Search::Players::Player_One, Search::EvaluationMethods::ModelSimulation, 40));
	PlayerPtr enemyPlayer(MicroSearch::Players::getPlayer(Search::Players::Player_Two, Search::PlayerModels::AttackClosest));

	// set up the game
	MicroSearch::Game g(state, selfPlayer, enemyPlayer, 1000);
	g.disp = &display;

	// play the game to the end
	g.play(true);
}

void ReplayVisualizer::setCombatUnits(MicroSearch::GameState & state, const BWAPI::Position & center, const int radius)
{
	state.setMaxUnits(200);

	int selfUnits = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getPlayer(0)->getUnits())
	{
		bool inRadius = unit->getDistance(center) < radius;

		if (selfUnits < 8 && inRadius && isCombatUnit(unit->getType()))
		{
			selfUnits++;
			state.addUnit(MicroSearch::Unit(unit, Search::Players::Player_One, BWAPI::Broodwar->getFrameCount()));
		}
		else
		{
			state.addNeutralUnit(MicroSearch::Unit(unit, Search::Players::Player_One, BWAPI::Broodwar->getFrameCount()));
		}
	}

	int enemyUnits = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getPlayer(1)->getUnits())
	{
		if (enemyUnits >= 8)
		{
			break;
		}

		bool inRadius = unit->getDistance(center) < radius;

		if (enemyUnits < 8 && inRadius && isCombatUnit(unit->getType()) && !unit->getType().isFlyer())
		{
			enemyUnits++;
			state.addUnit(MicroSearch::Unit(unit,Search::Players::Player_Two, BWAPI::Broodwar->getFrameCount()));
		}
		else
		{
			state.addNeutralUnit(MicroSearch::Unit(unit, Search::Players::Player_Two, BWAPI::Broodwar->getFrameCount()));
		}
	}

	int neutralUnits = 0;
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
	{
		neutralUnits++;

		const IDType player(getPlayer(unit->getPlayer()));

		if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field ||
			unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
		{
			state.addNeutralUnit(MicroSearch::Unit(unit, Search::Players::Player_None, BWAPI::Broodwar->getFrameCount()));
		}
	}

	state.finishedMoving();
}

bool ReplayVisualizer::isCombatUnit(BWAPI::UnitType type) const
{
	if (type == BWAPI::UnitTypes::Zerg_Lurker || type == BWAPI::UnitTypes::Protoss_Dark_Templar)
	{
		return false;
	}

	// no workers or buildings allowed
	if (type.isWorker())
	{
		return false;
	}

	// check for various types of combat units
	if (type.canAttack() || type == BWAPI::UnitTypes::Terran_Medic)
	{
		return true;
	}
		
	return false;
}

PlayerPtr ReplayVisualizer::getSearchPlayer(const IDType & playerToMoveMethod, const IDType & playerID, const IDType & evalMethod, const size_t & timeLimitMS)
{
	IDType bestResponseTo = Search::PlayerModels::No_Overkill_DPS;

	// base parameters to use in search
	MicroSearch::MicroSearchParameters baseParameters;
	baseParameters.setMaxPlayer(playerID);
	baseParameters.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
	baseParameters.setEvalMethod(evalMethod);
	baseParameters.setMaxDepth(Search::Constants::Max_Search_Depth);
	baseParameters.setScriptMoveFirstMethod(Search::PlayerModels::No_Overkill_DPS);
	baseParameters.setTimeLimit(timeLimitMS);
	
	// IF USING OPPONENT MODELING SET IT HERE
	baseParameters.setModelSimMethod(bestResponseTo);
	baseParameters.setPlayerModel(Search::Players::Player_Two, bestResponseTo, true);

	baseParameters.setPlayerToMoveMethod(playerToMoveMethod);
	return PlayerPtr(new MicroSearch::Player_AlphaBeta(playerID, baseParameters, MicroSearch::TTPtr(new MicroSearch::TranspositionTable())));
}

const MicroSearch::Unit ReplayVisualizer::getUnit(const UnitInfo & ui, const IDType & playerID) const
{
	BWAPI::UnitType type = ui.type;

	return MicroSearch::Unit(ui.type, MicroSearch::Position(ui.lastPosition.x(), ui.lastPosition.y()), ui.unitID, playerID, ui.lastHealth, 0,
		BWAPI::Broodwar->getFrameCount(), BWAPI::Broodwar->getFrameCount());	
}

const IDType ReplayVisualizer::getPlayer(BWAPI::Unit * unit) const
{
	return getPlayer(unit->getPlayer());
}

const IDType ReplayVisualizer::getPlayer(BWAPI::Player * player) const
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