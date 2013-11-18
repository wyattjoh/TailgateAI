#pragma once

#include "Common.h"
#include "BWAPI.h"
#include "GameCommander.h"
#include "..\..\AdversarialSearch\source\Timer.h"
#include "..\..\AdversarialSearch\source\GameState.h"
#include "..\..\AdversarialSearch\source\Player.h"
#include "..\..\AdversarialSearch\source\TranspositionTable.h"

#include <boost/shared_ptr.hpp>

class AnimData
{

public:

	bool	startedAttackAnim;
	int		startedAttackAnimTime;

	BWAPI::UnitCommandType	stopAttackCommand;
	int						stopAttackTime;

	AnimData()
		: startedAttackAnim(false)
		, startedAttackAnimTime(0) 
		, stopAttackCommand(BWAPI::UnitCommandTypes::None)
		, stopAttackTime(0)
	{
	}

	AnimData(bool started, int time)
		: startedAttackAnim(started)
		, startedAttackAnimTime(time)
		, stopAttackCommand(BWAPI::UnitCommandTypes::None)
		, stopAttackTime(0)
	{
	}

	AnimData(bool started, int time, BWAPI::UnitCommandType command, int stop)
		: startedAttackAnim(started)
		, startedAttackAnimTime(time)
		, stopAttackCommand(command)
		, stopAttackTime(stop)
	{
	}

};

class MicroSearchManager
{
	MicroSearch::SearchResults previousResults;
	MicroSearch::TTPtr TT;

	std::map<BWAPI::Unit *, AnimData> attackAnimData;

	bool gameOver;

	float initialTotalSqrt[2];

public:

	MicroSearchManager();

	MicroSearch::GameState	extractGameState();
		
	const IDType						getPlayerID(BWAPI::Player * player) const;
	const bool							isCombatUnit(BWAPI::Unit * unit) const;
	const MoveTuple						getMoveTuple(MicroSearch::GameState & state, const IDType & method);
	MicroSearch::Player *				getMicroSearchPlayer(const IDType & playerModel, const IDType & player) const;
	MicroSearch::MicroSearchParameters	getSearchParameters() const;

	void update();
	void onStart();
	
	void test();

	bool canIssueCommand(BWAPI::Unit * unit, const BWAPI::UnitCommand & command);

	bool canIssueMoveCommand(BWAPI::Unit * unit, BWAPI::Position targetPosition);
	bool canIssueStopCommand(BWAPI::Unit * unit);
	void doUnitMove(MicroSearch::GameState & currentState, MicroSearch::Unit & unit, MicroSearch::Move & move);
	
	const std::pair<int, int> getUnitCooldown(BWAPI::Unit * unit, MicroSearch::Unit & u) const;

	void smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target);
	void smartMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition);

	void drawCurrentState();
	void drawUnitMove(MicroSearch::GameState & currentState, MicroSearch::Unit & unit, MicroSearch::Move & move);
	void drawUnitCooldown(BWAPI::Unit * unit);
	void drawUnitHP(BWAPI::Unit * unit);
	void drawSearchResults(int x, int y);
	void drawAttackDebug();
};