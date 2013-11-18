#include "LargeGame.h"

using namespace MicroSearch;

LargeGame::LargeGame(const LargeGameState & initialState, PlayerPtr & p1, PlayerPtr & p2, const size_t & limit)
	: _numPlayers(0)
	, state(initialState)
	, _playerToMoveMethod(Search::PlayerToMove::Alternate)
	, rounds(0)
	, moveLimit(limit)
	, _storeHistory(false)
	, totalNodes(0)
	, totalDepthReached(0)
	, totalRoundsSearched(0)
{
	#ifdef OPENGL_VIS
		disp = NULL;
	#endif

	// add the players
	_players[Search::Players::Player_One] = p1;
	_players[Search::Players::Player_Two] = p2;

	for (size_t u1(0); u1 < Search::Constants::Max_Units + 1; ++u1)
	{
		for (size_t u2(0); u2 < Search::Constants::Max_Units + 1; ++u2)
		{
			totalNodesExpanded[u1][u2] = 0;
			totalNodesExpandedSq[u1][u2] = 0;
			totalDepth[u1][u2] = 0;
			totalDepthSq[u1][u2] = 0;
			totalRounds[u1][u2] = 0;
		}
	}
}

void LargeGame::storeHistory(const bool & store)
{
	_storeHistory = store;
	if (store)
	{
		history = new std::vector<LargeGameState>();
	}
}

// play the LargeGame until there is a winner
void LargeGame::play(bool printLargeGame)
{
	MoveTuple tuples[2] = {0, 0};

	t.start();

	// play until there is no winner
	while (!winner())
	{
		if (moveLimit && rounds > moveLimit)
		{
			break;
		}
		//printf("Playing round %d\n", rounds++);
	

		// the playr that will move next
		const IDType playerToMove(playerToMove());
		PlayerPtr & toMove = _players[playerToMove];
		PlayerPtr & enemy = _players[state.getEnemy(playerToMove)];

		// generate the moves possible from this state
		state.generateMoves(moves[toMove->ID()], toMove->ID());
	
		int numUnits1 = state.numUnits(Search::Players::Player_One);
		int numUnits2 = state.numUnits(Search::Players::Player_Two);
		
		// the tuple of moves he wishes to make
		tuples[toMove->ID()] = toMove->getMoveTuple(state, moves[toMove->ID()]);

		if (toMove->ID() == Search::Players::Player_One)
		{
			totalRoundsSearched++;
		}

		// if both players can move, generate the other player's moves
		if (state.bothCanMove())
		{
			state.generateMoves(moves[enemy->ID()], enemy->ID());
			tuples[enemy->ID()] = enemy->getMoveTuple(state, moves[enemy->ID()]);

			if (enemy->ID() == Search::Players::Player_One)
			{
				totalRoundsSearched++;
			}

			makeMoves(tuples[enemy->ID()], moves[enemy->ID()]);
		}
		
		// make the moves
		makeMoves(tuples[toMove->ID()], moves[toMove->ID()]);
		
		// gather stats about alpha-beta if it was used
		Player_AlphaBeta * ab = dynamic_cast<Player_AlphaBeta *>(_players[Search::Players::Player_One].get());
		
		if (ab)
		{
			totalNodesExpandedSq[numUnits1][numUnits2] += (ab->results().nodesExpanded * ab->results().nodesExpanded);
			totalNodesExpanded[numUnits1][numUnits2] += ab->results().nodesExpanded;
			totalDepth[numUnits1][numUnits2] += ab->results().maxDepthReached;
			totalDepthSq[numUnits1][numUnits2] += (ab->results().maxDepthReached * ab->results().maxDepthReached);
			totalRounds[numUnits1][numUnits2]++;
			totalNodes += ab->results().nodesExpanded;
			totalDepthReached += ab->results().maxDepthReached;
		}

		#ifdef OPENGL_VIS
			if (disp)
			{
				disp->SetState(state);
				if (ab)
				{
					disp->SetPlayerTypes(_players[0]->getType(), _players[1]->getType());
					disp->SetResults(ab->results());
					disp->SetParams(ab->params());
				}
				disp->OnFrame();
			}
		#endif

		state.finishedMoving();

		if (_storeHistory)
		{
			history->push_back(state);
		}
		
		rounds++;

		//state.print();

		// add the state to the history
		//_history.push_back(state);
	}

	LargeGameTimeMS = t.getElapsedTimeInMilliSec();
}

int LargeGame::getRounds()
{
	return rounds;
}

double LargeGame::getTime()
{
	return LargeGameTimeMS;
}


// causes playerToMove() to make the moves in the tuple
void LargeGame::makeMoves(const MoveTuple & tuple, MoveArray & arr)
{
	// for each simultaneous move in this tuple
	for (size_t u(0); u<arr.numUnitsInTuple(); ++u)
	{
		Move m = arr.getTupleMove(tuple, u);
		//printf("  Move (%d, %d) (%s, %s)\n", (int)tuple, (int)u, state.getUnit(m.player(), m.unit()).name().c_str(), m.moveString().c_str());
		state.makeMove(m, state);
	}
}

// determine the winner, NULL if there is none
bool LargeGame::winner()
{
	return state.winner() != Search::Players::Player_None; 
}

LargeGameState & LargeGame::getState()
{
	return state;
}

// determine the player to move
const IDType LargeGame::playerToMove()
{
	const IDType whoCanMove(state.whoCanMove());

	return (whoCanMove == Search::Players::Player_Both) ? Search::Players::Player_One : whoCanMove;
}
