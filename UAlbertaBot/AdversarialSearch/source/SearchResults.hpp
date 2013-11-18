#pragma once

#include <vector>

namespace MicroSearch
{
class SearchResults
{

public:

	bool 				solved,			// whether ot not a solution was found
						timedOut;		// did the search time-out?
	
	unsigned long long 	nodesExpanded;	// number of nodes expanded in the search
	
	double 				timeElapsed,	// time elapsed in milliseconds
						avgBranch;		// avg branching factor

	MoveTuple			bestMoveTuple;
	ScoreType			abValue;
	unsigned long long  ttcuts;
	size_t				maxDepthReached;	

	size_t				ttMoveOrders;
	size_t				ttFoundButNoMove;
	size_t				ttFoundNoCut;
	size_t				ttFoundCheck;
	size_t				ttFoundLessDepth;
	size_t				ttSaveAttempts;
	
	SearchResults() 
		: solved(false)
		, timedOut(false)
		, nodesExpanded(0)
		, timeElapsed(0)
		, avgBranch(0)
		, bestMoveTuple(0)
		, abValue(0)
		, ttcuts(0)
		, maxDepthReached(0)
		, ttMoveOrders(0)
		, ttFoundButNoMove(0)
		, ttFoundNoCut(0)
		, ttFoundCheck(0)
		, ttFoundLessDepth(0)
		, ttSaveAttempts(0)
	{
	}
};
}