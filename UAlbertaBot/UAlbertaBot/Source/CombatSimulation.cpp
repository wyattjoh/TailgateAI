#include "CombatSimulation.h"

CombatSimulation::CombatSimulation()
	: hasLogged(false)
{
	
}

// sets the starting states based on the combat units within a radius of a given position
// this center will most likely be the position of the forwardmost combat unit we control
void CombatSimulation::setCombatUnits(const BWAPI::Position & center, const int radius)
{
	MicroSearch::GameState s;
	s.setMaxUnits(100);

	BWAPI::Broodwar->drawCircleMap(center.x(), center.y(), 10, BWAPI::Colors::Red, true);

	std::vector<BWAPI::Unit *> ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;
	MapGrid::Instance().GetUnits(ourCombatUnits,   center, Options::Micro::COMBAT_REGROUP_RADIUS, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Options::Micro::COMBAT_REGROUP_RADIUS);


	int y = 0;

	BOOST_FOREACH (BWAPI::Unit * unit, ourCombatUnits)
	{
		if (InformationManager::Instance().isCombatUnit(unit->getType()))
		{
			s.addUnit(MicroSearch::Unit(unit, getPlayer(BWAPI::Broodwar->self()), BWAPI::Broodwar->getFrameCount()));
		}
	}

	y++;

	BOOST_FOREACH (UnitInfo ui, enemyCombatUnits)
	{
		if (!ui.type.isFlyer())
		{
			s.addUnit(getUnit(ui, getPlayer(BWAPI::Broodwar->enemy())));
		}
	}

	s.finishedMoving();

	state = s;
}

const MicroSearch::Unit CombatSimulation::getUnit(const UnitInfo & ui, const IDType & playerID) const
{
	BWAPI::UnitType type = ui.type;
	if (type == BWAPI::UnitTypes::Terran_Medic)
	{
		type = BWAPI::UnitTypes::Terran_Marine;
	}

	return MicroSearch::Unit(ui.type, MicroSearch::Position(ui.lastPosition.x(), ui.lastPosition.y()), ui.unitID, playerID, ui.lastHealth, 0,
		BWAPI::Broodwar->getFrameCount(), BWAPI::Broodwar->getFrameCount());	
}

std::pair<ScoreType, ScoreType> CombatSimulation::simulateCombat()
{
	MicroSearch::GameState s1(state);
	MicroSearch::GameState s2(state);

	MicroSearch::PlayerPtr selfChase(new MicroSearch::Player_NOK_AttackDPS(getPlayer(BWAPI::Broodwar->self())));
	MicroSearch::PlayerPtr selfKiter(new MicroSearch::Player_KiterDPS(getPlayer(BWAPI::Broodwar->self())));

	MicroSearch::PlayerPtr enemyChase(new MicroSearch::Player_AttackClosest(getPlayer(BWAPI::Broodwar->enemy())));
	MicroSearch::PlayerPtr enemyKiter(new MicroSearch::Player_KiterDPS(getPlayer(BWAPI::Broodwar->enemy())));

	MicroSearch::Game gameOptimistic (s1, selfChase, enemyChase, 1000);
	MicroSearch::Game gamePessimistic(s2, selfChase, enemyKiter, 1000);

	gameOptimistic.playScripts();
	gamePessimistic.playScripts();
	
	ScoreType evalOptimistic =  gameOptimistic.getState().eval(Search::Players::Player_One, Search::EvaluationMethods::SumDPS).val();
	ScoreType evalPessimistic = gamePessimistic.getState().eval(Search::Players::Player_One, Search::EvaluationMethods::SumDPS).val();

	BWAPI::Broodwar->drawTextScreen(240, 280, "Optimistic : %d", evalOptimistic);
	BWAPI::Broodwar->drawTextScreen(240, 295, "Pessimistic: %d", evalPessimistic);

	if (evalOptimistic > 1000000 || evalOptimistic < -1000000)
	{
		logState(gameOptimistic.getState());
	}

	return std::pair<ScoreType, ScoreType>(evalOptimistic, evalPessimistic);
}

const MicroSearch::GameState & CombatSimulation::getState() const
{
	return state;
}

const IDType CombatSimulation::getPlayer(BWAPI::Unit * unit) const
{
	return getPlayer(unit->getPlayer());
}

const IDType CombatSimulation::getPlayer(BWAPI::Player * player) const
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

void CombatSimulation::logState(const MicroSearch::GameState & state)
{
	if (hasLogged)
	{
		return;
	}

	std::stringstream log;

	log << "State: [EVAL=" << state.evalSumDPS(0) << ", SUMSQRT=(" << state.getTotalSumDPS(0) << "," << state.getTotalSumDPS(1) << ")\n";

	for (size_t p(0); p<Search::Constants::Num_Players; ++p)
	{
		log << "Player " << p << " units:\n";

		for (size_t u(0); u<state.numUnits(p); ++u)
		{
			const MicroSearch::Unit & unit(state.getUnit(p, u));

			log << "Unit " << u << ": " << unit.name() << " [HP=" << unit.currentHP() << ", X=" << unit.x() << ", Y=" << unit.y() << "]\n";
		}
	}

	Logger::Instance().log(log.str());

	hasLogged = true;
}