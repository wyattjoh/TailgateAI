#pragma once

#include "Common.h"
#include "LargeGameState.h"
#include "Player.h"
#include "Move.hpp"
#include <boost/shared_ptr.hpp>

#ifdef OPENGL_VIS
	#include "Display.h"
#endif

namespace MicroSearch
{
	
typedef	boost::shared_ptr<Player> PlayerPtr;

class LargeGame
{
protected:
	PlayerPtr			_players[2];
	size_t				_numPlayers;
	IDType				_playerToMoveMethod;
	size_t				rounds;
	Timer				t;
	double				LargeGameTimeMS;
	size_t				moveLimit;

	std::vector<LargeGameState> * history;
	bool				_storeHistory;

	LargeGameState state;

	// moves array to store moves in
	MoveArray moves[2];

public:
	
#ifdef OPENGL_VIS
	Display *			disp;
#endif

	long long unsigned	totalNodesExpanded[Search::Constants::Max_Units + 1][Search::Constants::Max_Units + 1];
	long long unsigned	totalNodesExpandedSq[Search::Constants::Max_Units + 1][Search::Constants::Max_Units + 1];
	size_t				totalDepth[Search::Constants::Max_Units + 1][Search::Constants::Max_Units + 1];
	size_t				totalDepthSq[Search::Constants::Max_Units + 1][Search::Constants::Max_Units + 1];
	size_t				totalRounds[Search::Constants::Max_Units + 1][Search::Constants::Max_Units + 1];

	long long unsigned	totalNodes;
	int					totalDepthReached;
	int					totalRoundsSearched;

	// LargeGame constructor
	LargeGame(const LargeGameState & initialState, PlayerPtr & p1, PlayerPtr & p2, const size_t & limit);

	// play the LargeGame until there is a winner
	void play(bool printLargeGame = false);

	// causes playerToMove() to make the moves in the tuple
	void makeMoves(const MoveTuple & tuple, MoveArray & moves);
	void storeHistory(const bool & store);

	// determine the winner, NULL if there is none
	bool winner();

	ScoreType LargeGame::eval(const IDType & evalMethod) const;

	LargeGameState & getState();
	int getRounds();
	double getTime();

	// determine the player to move
	const IDType LargeGame::playerToMove();
};

}