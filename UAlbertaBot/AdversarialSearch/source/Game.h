#pragma once

#include "Common.h"
#include "GameState.h"
#include "Player.h"
#include "Move.hpp"
#include <boost/shared_ptr.hpp>

#ifdef USING_VISUALIZATION_LIBRARIES
	#include "Display.h"
#endif

namespace MicroSearch
{
	
typedef	boost::shared_ptr<Player> PlayerPtr;

class Game
{
protected:
	PlayerPtr			_players[2];
	size_t				_numPlayers;
	IDType				_playerToMoveMethod;
	size_t				rounds;
	Timer				t;
	double				gameTimeMS;
	size_t				moveLimit;

	GameState state;

	// moves array to store moves in
	MoveArray moves[2];
	std::vector<Move> scriptMoves[2];

public:
	
#ifdef USING_VISUALIZATION_LIBRARIES
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

	// game constructor
	Game(const GameState & initialState, PlayerPtr & p1, PlayerPtr & p2, const size_t & limit);

	// play the game until there is a winner
	void play(bool printGame = false);
	void playScripts();

	// causes playerToMove() to make the moves in the tuple
	void makeMoves(const MoveTuple & tuple, MoveArray & moves);
	void makeMoves(const std::vector<Move> & moveVec);
	void storeHistory(const bool & store);

	// determine the winner, NULL if there is none
	bool gameOver();

	ScoreType Game::eval(const IDType & evalMethod) const;

	GameState & getState();
	int getRounds();
	double getTime();

	// determine the player to move
	const IDType Game::playerToMove();
};

}