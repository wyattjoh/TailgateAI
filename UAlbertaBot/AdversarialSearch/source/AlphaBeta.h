#pragma once

#include <limits>

#include "Timer.h"
#include "GameState.h"
#include "SearchResults.hpp"
#include "Move.hpp"
#include "GraphViz.hpp"
#include "Array.hpp"
#include "MoveArray.hpp"
#include "TranspositionTable.h"
#include "MicroSearchParameters.h"
#include "Player.h"

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

namespace MicroSearch
{
typedef	boost::shared_ptr<TranspositionTable> TTPtr;

class Game;
class MicroSearchParameters;
class Player;

class AlphaBeta
{
	MicroSearchParameters	_params;
	SearchResults			_results;
	MicroSearch::Timer		_searchTimer;

	size_t					_currentRootDepth;

	// an array of MoveArrays to store all moves possible at a given depth
	Array<MoveArray, Search::Constants::Max_Search_Depth>			_allMoves;

	Array2D<MoveTuple, 
			Search::Constants::Max_Search_Depth, 
			Search::Constants::Max_Ordered_Moves>					_orderedMoves;

	std::vector<PlayerPtr>											_allScripts;

	TTPtr _TT;

public:

	AlphaBeta(const MicroSearchParameters & params, TTPtr TT);

	void doSearch(GameState & initialState);

	// search functions
	AlphaBetaValue IDAlphaBeta(GameState & initialState, const size_t & maxDepth);
	AlphaBetaValue alphaBeta(GameState & state, size_t depth, const IDType lastPlayerToMove, const MoveTuple * firstSimMove, AlphaBetaScore alpha, AlphaBetaScore beta);

	// Transposition Table
	TTLookupValue TTlookup(const GameState & state, AlphaBetaScore & alpha, AlphaBetaScore & beta, const size_t & depth);
	void TTsave(GameState & state, const AlphaBetaScore & value, const AlphaBetaScore & alpha, const AlphaBetaScore & beta, const size_t & depth, 
				const IDType & firstPlayer, const AlphaBetaMove & bestFirstMove, const AlphaBetaMove & bestSecondMove);

	void doTupleMoves(GameState & state, MoveArray & moves, const MoveTuple & tuple);

	// Transposition Table look up + alpha/beta update

	// get the results from the search
	const SearchResults & getResults() const;

	// get a new player based on a player model
	MoveTuple getMoveTuple(GameState & state, IDType & player) const;
	
	void generateOrderedMoves(GameState & state, MoveArray & moves, const TTLookupValue & TTval, const IDType & playerToMove, const size_t & depth);
	const IDType getEnemy(const IDType & player) const;
	const IDType getPlayerToMove(GameState & state, const size_t & depth, const IDType & lastPlayerToMove, const bool isFirstSimMove) const;
	const MoveTuple getNextMoveTuple(const MoveTuple & t, const size_t & depth) const;
	const MoveTuple getNumMoveTuples(MoveArray & moves, const TTLookupValue & TTval, const IDType & playerToMove, const size_t & depth) const;
	const AlphaBetaMove & getAlphaBetaMove(const TTLookupValue & TTval, const IDType & playerToMove) const;
	const bool pruneMove(GameState & state, const IDType & playerToMove, const MoveArray & moves, const MoveTuple & move) const;
	const bool searchTimeOut();
	const bool isRoot(const size_t & depth) const;
	const bool terminalState(GameState & state, const size_t & depth) const;
	const bool isTranspositionLookupState(GameState & state, const MoveTuple * firstSimMove) const;

	void printTTResults() const;
};
}