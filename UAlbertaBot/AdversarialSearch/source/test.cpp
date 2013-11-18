#include "BWAPI.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>

#include "Common.h"
#include "MicroSearchParameters.h"
#include "Game.h"
#include "Player.h"
#include "GameState.h"
#include "Timer.h"
#include "AlphaBeta.h"
#include "TranspositionTable.h"

#ifdef USING_VISUALIZATION_LIBRARIES
	#include "Display.h"
#endif

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace MicroSearch;

Map * map = NULL;

GameState getInitialState()
{
	GameState state;

	// add units for player one
	state.addUnit(1, BWAPI::UnitTypes::Terran_Missile_Turret, Search::Players::Player_One, Position(100,328));
	//state.addUnit(1, BWAPI::UnitTypes::Protoss_Dragoon, Search::Players::Player_One, Position(200,300));
	//state.addUnit(1, BWAPI::UnitTypes::Protoss_Dragoon, Search::Players::Player_One, Position(200,400));
	//state.addUnit(1, BWAPI::UnitTypes::Protoss_Dragoon, Search::Players::Player_One, Position(100,110));
	//state.addUnit(1, BWAPI::UnitTypes::Protoss_Zealot, Search::Players::Player_One, Position(200,150));
	//state.addUnit(1, BWAPI::UnitTypes::Terran_Marine, Search::Players::Player_One, Position(0,0));
	
	// add units for player two	
	state.addUnit(1, BWAPI::UnitTypes::Terran_Missile_Turret, Search::Players::Player_Two, Position(1700,328));
	//state.addUnit(1, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, Position(300,150));
	//state.addUnit(1, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, Position(300,250));
	//state.addUnit(1, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, Position(300,350));
	//state.addUnit(1, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, Position(300,450));
	//state.addUnit(1, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, Position(300,550));
	//state.addUnit(1, BWAPI::UnitTypes::Terran_Marine, Search::Players::Player_Two, Position(0,0));
	state.finishedMoving();
	
	//state.print();

	return state;
}

Position getRandomPosition(const PositionType & xlimit, const PositionType & ylimit)
{
	int x = xlimit - (rand() % (2*xlimit));
	int y = ylimit - (rand() % (2*ylimit));

	return Position(x, y);
}

GameState getSymmetricState(	const IDType & playerToMoveMethod, 
								const BWAPI::UnitType typeUnit1, const size_t & numUnit1, 
								const BWAPI::UnitType typeUnit2, const size_t & numUnit2, 
								const PositionType & xLimit, const PositionType & yLimit)
{
	GameState state;
	state.setMap(map);
	//state.setMaxUnits(12);

	for (size_t u(0); u<numUnit1; ++u)
	{
		state.addUnitSymmetric(1, typeUnit1, Search::Players::Player_One, getRandomPosition(xLimit, yLimit));
	}

	for (size_t u(0); u<numUnit2; ++u)
	{
		state.addUnitSymmetric(1, typeUnit2, Search::Players::Player_One, getRandomPosition(xLimit, yLimit));
	}

	state.finishedMoving();

	return state;
}

void generateAllStates(	std::vector<GameState> & states, const size_t & numEachState,
						const BWAPI::UnitType typeUnit1, const size_t & maxNumUnit1, 
						const BWAPI::UnitType typeUnit2, const size_t & maxNumUnit2, 
						const PositionType & xLimit, const PositionType & yLimit)
{	
	// for all combinations of these unit types
	for (size_t n1(0); n1<=maxNumUnit1; n1++)
	{
		for (size_t n2(0); n2<=maxNumUnit2; n2++)
		{
			// if we have at least one unit to add
			if (n1 + n2 != 0)
			{
				// for each instance of this state we want
				// the starting positions will all be different
				for (size_t n(0); n<numEachState; ++n)
				{
					// add the state to the vector
					states.push_back(getSymmetricState(Search::PlayerToMove::Alternate, typeUnit1, n1, typeUnit2, n2, xLimit, yLimit));
				}
			}
		}
	}
}

MicroSearchParameters getParams()
{
	MicroSearchParameters params;
	params.setMaxPlayer(Search::Players::Player_One);
	//params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::AttackClosest, true);
	params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
	params.setEvalMethod(Search::EvaluationMethods::ModelSimulation);
	params.setMaxDepth(30);
	params.setTimeLimit(55);

	return params;
}

void testScripts()
{
	std::vector<PlayerPtr> allScripts;
	allScripts.push_back(PlayerPtr(new Player_AttackClosest	(0)));
	allScripts.push_back(PlayerPtr(new Player_AttackWeakest	(0)));
	allScripts.push_back(PlayerPtr(new Player_AttackDPS		(0)));
	allScripts.push_back(PlayerPtr(new Player_Kiter			(0)));
	allScripts.push_back(PlayerPtr(new Player_KiterDPS		(0)));
	allScripts.push_back(PlayerPtr(new Player_NOK_AttackDPS	(0)));

	GameState state = getInitialState();
	MoveArray moves;
	state.generateMoves(moves, 0);

	for (size_t s(0); s<allScripts.size(); s++)
	{
		MoveTuple scriptMove = allScripts[s]->getMoveTuple(state, moves);
		printf("Script %d produced MoveTuple %d", (int)s, (int)scriptMove);
		state.printMoveTuple(0, scriptMove);
	}
}

void testIDA(const size_t & timeLimitMS, const size_t & maxDepth)
{
	MicroSearchParameters params;
	params.setMaxPlayer(Search::Players::Player_One);
	params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
	params.setPlayerToMoveMethod(Search::PlayerToMove::Alternate);
	params.setEvalMethod(Search::EvaluationMethods::SumHP);
	params.setModelSimMethod(Search::PlayerModels::AttackClosest);
	params.setMaxDepth(maxDepth);
	params.setTimeLimit(timeLimitMS);
	params.setScriptMoveFirstMethod(Search::PlayerModels::No_Overkill_DPS);
	//params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::AttackClosest, true);

	AlphaBeta ab(params, TTPtr(new TranspositionTable()));

	GameState s = getInitialState();
	ab.doSearch(s);

	s.printMoveTuple(params.maxPlayer(), ab.getResults().bestMoveTuple);
}

PlayerPtr getSearchPlayer(const IDType & playerToMoveMethod, const IDType & playerID, const IDType & evalMethod, const size_t & timeLimitMS)
{
	IDType bestResponseTo = Search::PlayerModels::No_Overkill_DPS;

	// base parameters to use in search
	MicroSearchParameters baseParameters;
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
	return PlayerPtr(new Player_AlphaBeta(playerID, baseParameters, TTPtr((TranspositionTable *)NULL)));
}

void addSearchPlayers50ms(std::vector<PlayerPtr> & players, const IDType & playerID)
{
	players.push_back(getSearchPlayer(Search::PlayerToMove::Alternate, playerID, Search::EvaluationMethods::ModelSimulation, 50));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Not_Alternate, playerID, Search::EvaluationMethods::ModelSimulation));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Not_Alternate, playerID, Search::EvaluationMethods::ModelSimulation));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Random, playerID, Search::EvaluationMethods::ModelSimulation));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Max_Player, playerID));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Min_Player, playerID));
}

void addSearchPlayers5ms(std::vector<PlayerPtr> & players, const IDType & playerID)
{
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Alternate, playerID, Search::EvaluationMethods::ModelSimulation, 5));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Not_Alternate, playerID, Search::EvaluationMethods::ModelSimulation, 5));
	players.push_back(getSearchPlayer(Search::PlayerToMove::Random, playerID, Search::EvaluationMethods::ModelSimulation, 5));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Random, playerID, Search::EvaluationMethods::ModelSimulation));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Max_Player, playerID));
	//players.push_back(getSearchPlayer(Search::PlayerToMove::Min_Player, playerID));
}

void addModelPlayers(std::vector<PlayerPtr> & players, const IDType & playerID)
{
	players.push_back(PlayerPtr(new Player_AttackClosest	(playerID)));
	players.push_back(PlayerPtr(new Player_AttackWeakest	(playerID)));
	players.push_back(PlayerPtr(new Player_AttackDPS		(playerID)));
	players.push_back(PlayerPtr(new Player_Kiter			(playerID)));
	players.push_back(PlayerPtr(new Player_KiterDPS			(playerID)));
	players.push_back(PlayerPtr(new Player_NOK_AttackDPS	(playerID)));
	players.push_back(PlayerPtr(new Player_Random			(playerID)));
}

void results()
{
	// vector of all states we will test
	std::vector<GameState> states;
	generateAllStates(states, 1, BWAPI::UnitTypes::Terran_Marine, 4, BWAPI::UnitTypes::Terran_Medic, 4, 128, 128);
	generateAllStates(states, 1, BWAPI::UnitTypes::Protoss_Dragoon, 4, BWAPI::UnitTypes::Protoss_Zealot, 4, 128, 128);
	generateAllStates(states, 1, BWAPI::UnitTypes::Terran_Marine, 4, BWAPI::UnitTypes::Protoss_Dragoon, 4, 128, 128);
	generateAllStates(states, 1, BWAPI::UnitTypes::Terran_Marine, 4, BWAPI::UnitTypes::Zerg_Zergling, 4, 128, 128);

	// vector of all players
	std::vector<PlayerPtr> playerOnePlayers;
	std::vector<PlayerPtr> playerTwoPlayers;
	
	addSearchPlayers50ms(playerOnePlayers, Search::Players::Player_One);
	addModelPlayers(playerTwoPlayers, Search::Players::Player_Two);
	
#ifdef USING_VISUALIZATION_LIBRARIES
	Display disp(map ? map->getWidth() / 4 : 64, map ? map->getHeight() / 4 : 64);
	disp.OnStart();
	disp.LoadMapTexture(map, 19);
#endif

	printf("   P1    P2    ST   UNIT      EVAL    RND           MS\n");
	// for each player one player
	for (size_t p1Player(0); p1Player < playerOnePlayers.size(); p1Player++)
	{
		// for each player two player
		for (size_t p2Player(0); p2Player < playerTwoPlayers.size(); p2Player++)
		{
			// for each state we care about
			for (size_t state(0); state < states.size(); ++state)
			{
				fprintf(stderr, "%5d %5d %5d %5d", (int)p1Player, (int)p2Player, (int)state, (int)states[state].numUnits(Search::Players::Player_One));
				printf("%5d %5d %5d %5d", (int)p1Player, (int)p2Player, (int)state, (int)states[state].numUnits(Search::Players::Player_One));
				
				// get player one
				PlayerPtr playerOne(playerOnePlayers[p1Player]);

				// give it a new transposition table if it's an alpha beta player
				Player_AlphaBeta * p1AB = dynamic_cast<Player_AlphaBeta *>(playerOne.get());
				if (p1AB)
				{
					p1AB->setTranspositionTable(TTPtr(new TranspositionTable()));
				}

				// get player two
				PlayerPtr playerTwo(playerTwoPlayers[p2Player]);
				Player_AlphaBeta * p2AB = dynamic_cast<Player_AlphaBeta *>(playerTwo.get());
				if (p2AB)
				{
					p2AB->setTranspositionTable(TTPtr(new TranspositionTable()));
				}

				// construct the game
				Game g(states[state], playerOne, playerTwo, 500);
				#ifdef USING_VISUALIZATION_LIBRARIES
					g.disp = &disp;
				#endif

				// play the game to the end
				g.play(true);
				
				ScoreType gameEval = g.getState().eval(Search::Players::Player_One, Search::EvaluationMethods::SumDPS).val();

				printf(" %10d %6d %12.2lf %llu %lf", gameEval, g.getRounds(), g.getTime(), g.totalNodes, (double)g.totalDepthReached/g.totalRoundsSearched);

				for (size_t u1(1); u1 < Search::Constants::Max_Units + 1; ++u1)
				{
					for (size_t u2(1); u2 < Search::Constants::Max_Units + 1; ++u2)
					{
						//printf("%7d %14d %14d %17llu %17llu", g.totalRounds[u1][u2], g.totalDepth[u1][u2], g.totalDepthSq[u1][u2], g.totalNodesExpanded[u1][u2], g.totalNodesExpandedSq[u1][u2]);
					}
				}

				fprintf(stderr, "%12d\n", gameEval);

				printf("\n");
			}
		}
	}
}

void printStats(BWAPI::UnitType t)
{
	Unit u(t, 0, 0, Position(0,0));
	printf("%s %d %d %d %d %d %lf\n", t.getName().c_str(), u.maxHP(), u.damage(), u.range(), u.attackCooldown(), u.attackInitFrameTime(), u.speed());
}

int main(int argc, char *argv[])
{
	Search::StarcraftData::init();
	MicroSearch::Hash::initHash();

	map = new Map;
	map->load("C:\\test.txt");

	//testIDA(550000, 20);
	//testScripts();
	results();

    return 0;
}
