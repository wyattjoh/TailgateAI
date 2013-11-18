#pragma once

#include "Common.h"
#include "Move.hpp"

namespace MicroSearch
{
class MoveArray
{
	// the array which contains all the moves
	Array2D<Move, Search::Constants::Max_Units, Search::Constants::Max_Moves> _moves;

	// how many moves each unit has
	Array<size_t, Search::Constants::Max_Units> _numMoves;

	// running product of _numMoves[i] from right to left
	Array<MoveTuple, Search::Constants::Max_Units> _numMovesProduct;

	// the number of units that have moves;
	size_t _numUnits;
	size_t _maxUnits;

	// the number of unit tuples
	MoveTuple _numMoveTuples;

public:

	MoveArray(const size_t maxUnits = 0) 
		: _numUnits(0)
		, _maxUnits(Search::Constants::Max_Units)
		, _numMoveTuples(0)
	{
		_numMoves.fill(0);
	}

	void resize(const size_t & size)
	{
		_maxUnits = size;
		_moves.resize(size, size + Search::Constants::Num_Directions + 1);
		_numMoves.resize(size);
		_numMovesProduct.resize(size);

		_numMoves.fill(0);
	}

	MoveArray & operator = (const MoveArray & rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}

		_moves = rhs._moves;
		_numMoves = rhs._numMoves;
		_numMovesProduct = rhs._numMovesProduct;
		_numUnits = rhs._numUnits;
		_maxUnits = rhs._maxUnits;
		_numMoveTuples = rhs._numMoveTuples;

		return *this;
	}

	
	void clear() 
	{
		_numUnits = 0;
		_numMoveTuples = 0;
		_numMoves.fill(0);
	}

	// returns a given move from a unit
	const Move & getMove(const size_t & unit, const MoveTuple & move) const
	{
		Move m = _moves[unit][(size_t)move];

		if (m.unit() == 255)
		{
			fprintf(stderr, "Returning a bad unit, check this shit out\n");
		}

		return _moves[unit][(size_t)move];
	}

	const size_t maxUnits() const
	{
		return _moves.getRows();
	}

	// adds a Move to the unit specified
	void add(const Move & move)
	{
		_moves[move._unit][_numMoves[move._unit]] = move;
		_numMoves[move._unit]++;

		calculateNumMoveTuples();
		validateMoves();
	}
	
	// calculates the number of move tuples there are
	// as well as the product array for calculating tuple indices
	void calculateNumMoveTuples()
	{
		// calculate the running product from right to left for base number conversino
		_numMovesProduct[_numUnits - 1] = _numMoves[_numUnits - 1];

		// from n-2 to 0, update product
		for (size_t u(1); u < _numUnits; ++u)
		{
			// update the product as itself times the element to the right

			_numMovesProduct[_numUnits - u - 1] = _numMoves[_numUnits - u - 1] * _numMovesProduct[_numUnits - u];
		}

		// update the number of tuples variable
		_numMoveTuples = _numMovesProduct[0];
	}

	// returns the Move associated with (Tuple, Unit)
	// Essentially calculates the value of radix 'unitInTuple' if _numMoves[] is considered a radix for counting
	const Move & getTupleMove(const MoveTuple & tuple, const size_t & unitInTuple) const
	{
		// yeah this would cause some problems
	
		if (tuple >= numMoveTuples())
		{
			printf("WTF TOO MANY TUPLES %d %d %d\n", (int)tuple, (int)unitInTuple, (int)_numMoveTuples);
			exit(-1);
		}

		assert(tuple < numMoveTuples());

		// if there is only one unit then the tuple correspond to its move
		// so just return it to save time
		if (_numUnits == 1)
		{
			return getMove(unitInTuple, tuple);
		}

		// running remainder for calculation
		MoveTuple remainder = tuple;

		// holds some temp data about remainder
		MoveTuple quotient(0);

		// from left to right starting at 1
		for (size_t i(1); i<_numUnits; ++i)
		{
			// the current quotient and remainder
			quotient		=  remainder / _numMovesProduct[i];
			remainder		-= quotient * _numMovesProduct[i];
			
			// if this is the tuple that we are concerned about
			if (unitInTuple == (i - 1))
			{
				// return the corresponding move
				return getMove(unitInTuple, quotient);
			}
		}

		// if we have not yet returned a value, then it is the last index, so return the remainder
		return getMove(unitInTuple, remainder);
	}

	bool validateMoves()
	{
		for (size_t u(0); u<numUnits(); ++u)
		{
			for (size_t m(0); m<numMoves(u); ++m)
			{
				const Move & move(getMove(u, m));

				if (move.unit() > 200)
				{
					printf("Unit Move Incorrect! Something will be wrong\n");
					return false;
				}
			}
		}
		
		return true;
	}

	const IDType getUnitID(const IDType & unit) const
	{
		return getMove(unit, 0).unit();
	}

	const IDType getPlayerID(const IDType & unit) const
	{
		return getMove(unit, 0).player();
	}

	void addUnit() 											{ _numUnits++; }

	const size_t & numUnits()						const	{ return _numUnits; }
	const MoveTuple & numMoveTuples()				const	{ return _numMoveTuples; }
	const size_t & numUnitsInTuple()				const	{ return numUnits(); }
	const size_t & numMoves(const size_t & unit)	const	{ return _numMoves[unit]; }
	const MoveTuple getProduct(const size_t & unit)	const	{ return (unit == (_numUnits-1)) ? 1 : _numMovesProduct[unit+1]; }
};
}