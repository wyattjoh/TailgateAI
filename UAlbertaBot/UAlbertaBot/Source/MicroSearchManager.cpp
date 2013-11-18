#include "Common.h"
#include "MicroSearchManager.h"

MicroSearchManager::MicroSearchManager() 
	: TT(new MicroSearch::TranspositionTable())
	, gameOver(false)
	
{
	Search::StarcraftData::init();

	initialTotalSqrt[0] = 0;
	initialTotalSqrt[1] = 0;
}

const MoveTuple	MicroSearchManager::getMoveTuple(MicroSearch::GameState & state, const IDType & playerModel)
{
	const IDType playerID = getPlayerID(BWAPI::Broodwar->self());

	//MicroSearch::GameState state = extractGameState();
	//state.addUnit(2, BWAPI::UnitTypes::Protoss_Dragoon, Search::Players::Player_One, MicroSearch::Position(0,0));
	//state.addUnit(2, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, MicroSearch::Position(0,0));
	//state.finishedMoving();

	state.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[1]);

	MicroSearch::MicroSearchParameters params;
	params.setMaxPlayer(Search::Players::Player_One);
	params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
	params.setPlayerToMoveMethod(Search::PlayerToMove::Alternate);
	params.setEvalMethod(Search::EvaluationMethods::ModelSimulation);
	//params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::AttackClosest, true);
	params.setScriptMoveFirstMethod(Search::PlayerModels::No_Overkill_DPS);
	params.setMaxDepth(50);
	params.setTimeLimit(2);

	MicroSearch::AlphaBeta ab(params, TT);;

	//ab.doSearch(state);

	// get the player based on the player model
	boost::shared_ptr<MicroSearch::Player> player(new MicroSearch::Player_Kiter(playerID));
	
	MicroSearch::MoveArray moves;
	state.generateMoves(moves, player->ID());
	MoveTuple nm = player->getMoveTuple(state, moves);

	// retrieve the best move tuple
	MoveTuple m = ab.getResults().bestMoveTuple;

	MicroSearch::GameState s(state);
	s.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[1]);

	BWAPI::Broodwar->drawTextScreen(100, 100, "Value: %d", ab.getResults().abValue);
	BWAPI::Broodwar->drawTextScreen(100, 120, "Value: %d", s.evalSumDPS(0));

	BWAPI::Broodwar->drawTextScreen(200, 200, "Initial Sqrt: %f %f", initialTotalSqrt[0], initialTotalSqrt[1]);
	BWAPI::Broodwar->drawTextScreen(200, 220, "Current Sqrt: %f %f", state.getTotalSumDPS(0), state.getTotalSumDPS(1));

	//BWAPI::Broodwar->printf("Best Move Tuple %d - %d - %d %d %d %d", (int)m, (int)nm, (int)ab.getResults().nodesExpanded, (int)ab.getResults().maxDepthReached, (int)ab.getResults().abValue, (int)state.evalSumDPS(0));

	// return the move tuple
	return nm;
}

void MicroSearchManager::onStart()
{
	MicroSearch::GameState state(extractGameState());

	initialTotalSqrt[0] = state.getTotalSumDPS(0);
	initialTotalSqrt[1] = state.getTotalSumDPS(1);
}

// get an abstract GameState object from the current BWAPI state
MicroSearch::GameState MicroSearchManager::extractGameState()
{
	// construct a state with the current time
	MicroSearch::GameState state;
	state.setTime(BWAPI::Broodwar->getFrameCount());

	// add each of our fighting units
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (isCombatUnit(unit))
		{
			MicroSearch::Unit u(unit, getPlayerID(unit->getPlayer()), BWAPI::Broodwar->getFrameCount());
			std::pair<int, int> cd = getUnitCooldown(unit, u);
			u.setCooldown(cd.first, cd.second);
			state.addUnit(u);
		}
	}

	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
	{
		if (isCombatUnit(unit))
		{
			MicroSearch::Unit u(unit, getPlayerID(unit->getPlayer()), BWAPI::Broodwar->getFrameCount());
			u.setCooldown(BWAPI::Broodwar->getFrameCount(), BWAPI::Broodwar->getFrameCount() + unit->getGroundWeaponCooldown());
			state.addUnit(u);
		}
	}

	if (state.numUnits(0) == 0 && state.numUnits(1) == 0)
	{
		gameOver = true;
		Logger::Instance().log("0\n");
	}
	else if (state.numUnits(1) == 0)
	{
		gameOver = true;
		Logger::Instance().log("1\n");
	}
	else if (state.numUnits(0) == 0)
	{
		gameOver = true;
		Logger::Instance().log("-1\n");
	}

	if (BWAPI::Broodwar->getFrameCount() > 5000)
	{
		std::stringstream ss;
		state.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[0]);
		ss << state.evalSumDPS(0) << "\n";

		gameOver = true;
		Logger::Instance().log(ss.str());
	}

	if (gameOver)
	{	
		BWAPI::Broodwar->restartGame();
	}

	state.finishedMoving();
	return state;
}

void MicroSearchManager::test()
{
	BWAPI::Unit * unit = BWAPI::Broodwar->getUnit(3);

	int x = unit->getPosition().x();
	int y = unit->getPosition().y();

	unit->move(BWAPI::Position(x + 32, y));
}

void MicroSearchManager::update()
{
	if (!gameOver)
	{
		// for each unit that we control, do some book keeping
		BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
		{
			// if it is a combat unit
			if (isCombatUnit(unit))
			{
				if (unit->getTarget())
				BWAPI::Broodwar->drawLineMap(unit->getPosition().x()-2, unit->getPosition().y()-2,
					unit->getTarget()->getPosition().x(), unit->getTarget()->getPosition().y(), BWAPI::Colors::White);

				// if it's not in our map, add a default entry
				if ((attackAnimData.find(unit) == attackAnimData.end()))
				{
					attackAnimData[unit] = AnimData(false, 0);
				}

				// if it is attacking and it wasn't attacking last frame
				if (unit->isAttackFrame() && !attackAnimData[unit].startedAttackAnim)
				{
					// update the map to reflect that it has started its attack animation
					attackAnimData[unit] = AnimData(true, BWAPI::Broodwar->getFrameCount());
				}

				BWAPI::Broodwar->drawTextMap(unit->getPosition().x()-10, unit->getPosition().y()+10, attackAnimData[unit].startedAttackAnim ? "T" : "F");
			}
		}

		// draw the state for debugging
		drawCurrentState();

		drawSearchResults(250,250);
	}

	//drawAttackDebug();
}


void MicroSearchManager::drawAttackDebug()
{
	char * trueFix = "\x07";
	char * falseFix = "\x06";

	BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		int x = unit->getPosition().x();
		int y = unit->getPosition().y() + 9;

		BWAPI::Broodwar->drawTextMap(x, y, "%s isAttacking", unit->isAttacking() ? trueFix : falseFix);
		BWAPI::Broodwar->drawTextMap(x, y+10, "%s isAttackFrame", unit->isAttackFrame() ? trueFix : falseFix);
		BWAPI::Broodwar->drawTextMap(x, y+20, "%s isMoving", unit->isMoving() ? trueFix : falseFix);
	}
}

const std::pair<int, int> MicroSearchManager::getUnitCooldown(BWAPI::Unit * unit, MicroSearch::Unit & u) const
{
	int attackCooldown(0);
	int moveCooldown(0);

	BWAPI::UnitCommand lastCommand = unit->getLastCommand();
	int lastCommandFrame = unit->getLastCommandFrame();
	int currentFrame = BWAPI::Broodwar->getFrameCount();
	int framesSinceCommand = currentFrame - lastCommandFrame;

	attackCooldown = BWAPI::Broodwar->getFrameCount() + std::max(0, unit->getGroundWeaponCooldown()-2);

	// if the last attack was an attack command
	if (lastCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit)
	{
		moveCooldown = BWAPI::Broodwar->getFrameCount() + std::max(0, u.attackInitFrameTime() - framesSinceCommand);

		//BWAPI::Broodwar->drawTextScreen(100,100, "%d, %d", attackCooldown-currentFrame, moveCooldown-currentFrame);
	}
	// if the last command was a move command
	else if (lastCommand.getType() == BWAPI::UnitCommandTypes::Move)
	{
		moveCooldown = currentFrame;
	}

	if (moveCooldown - BWAPI::Broodwar->getFrameCount() < 4 || unit->isMoving())
	{
		moveCooldown = currentFrame;
	}

	moveCooldown = std::min(moveCooldown, attackCooldown);

	return std::pair<int, int>(attackCooldown, moveCooldown);
}

void MicroSearchManager::drawUnitMove(MicroSearch::GameState & currentState, MicroSearch::Unit & unit, MicroSearch::Move & move)
{
	IDType enemyPlayer = (unit.player() + 1) % 2;

	if (move._moveType == MicroSearch::MoveTypes::ATTACK)
	{
		MicroSearch::Unit & enemyUnit(currentState.getUnit(enemyPlayer,move._moveIndex));
		
		BWAPI::Broodwar->drawLineMap(unit.x(), unit.y(), enemyUnit.x(), enemyUnit.y(), BWAPI::Colors::Cyan);
	}
	else if (move._moveType == MicroSearch::MoveTypes::MOVE)
	{
		MicroSearch::Position pos(Search::Constants::Move_Dir[move._moveIndex][0], Search::Constants::Move_Dir[move._moveIndex][1]);
		
		// tell the unit to move
		MicroSearch::Position dest(unit.x() + (pos.x() * 32), unit.y() + (pos.y() * 32));
		BWAPI::Broodwar->drawLineMap(unit.x(), unit.y(), dest.x(), dest.y(), BWAPI::Colors::Yellow);
	}
}

void MicroSearchManager::smartAttackUnit(BWAPI::Unit * attacker, BWAPI::Unit * target)
{
	assert(attacker && target);
	AnimData data;
	if (attackAnimData.find(attacker) != attackAnimData.end())
	{
		data = attackAnimData[attacker];
	}

	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());
	bool isAttackFrame = (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit);

	// if we have issued a command to this unit already this frame, ignore this one
	if ((isAttackFrame && (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())) || attacker->isAttackFrame())
	{
		//BWAPI::Broodwar->printf("Already issued attack this frame");
		return;
	}

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
		//BWAPI::Broodwar->printf("Already attacking this target %d", BWAPI::Broodwar->getFrameCount());
		return;
	}

	if (currentCommand.type == BWAPI::UnitCommandTypes::Attack_Unit)
	{
		// this will be false if we have not yet started animating (turning, moving, etc)
		if (data.startedAttackAnim == false)
		{
			return;
		}
		else
		{
			int framesSinceAnim = BWAPI::Broodwar->getFrameCount() - data.startedAttackAnimTime;

			// TODO: add logic for init/rpt move
			if (framesSinceAnim < Search::StarcraftData::getAttackFrames(attacker->getType()).first)
			{
				//BWAPI::Broodwar->printf("Not done animating yet");
				return;		
			}
		}
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);

	BWAPI::Broodwar->drawLineMap(	attacker->getPosition().x()-3, attacker->getPosition().y()-3,
									target->getPosition().x()-3, target->getPosition().y()-3,
									BWAPI::Colors::Red );

}

void MicroSearchManager::smartMove(BWAPI::Unit * attacker, BWAPI::Position targetPosition)
{
	assert(attacker);

	if (!canIssueMoveCommand(attacker, targetPosition))
	{
		return;
	}

	attackAnimData[attacker] = AnimData(false, BWAPI::Broodwar->getFrameCount(), BWAPI::UnitCommandTypes::Move, BWAPI::Broodwar->getFrameCount());
	attacker->move(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) 
	{
		BWAPI::Broodwar->drawLineMap(attacker->getPosition().x(), attacker->getPosition().y(),
									 targetPosition.x(), targetPosition.y(), BWAPI::Colors::Orange);
	}
}


bool MicroSearchManager::canIssueMoveCommand(BWAPI::Unit * unit, BWAPI::Position targetPosition)
{
	AnimData data;
	if (attackAnimData.find(unit) != attackAnimData.end())
	{
		data = attackAnimData[unit];
	}

	BWAPI::UnitCommand lastCommand = unit->getLastCommand();

	if (lastCommand.type == BWAPI::UnitCommandTypes::Attack_Unit)
	{
		BWAPI::Broodwar->drawCircleMap(lastCommand.getTarget()->getPosition().x(), lastCommand.getTarget()->getPosition().y(), 5, BWAPI::Colors::Cyan, true);

		if (BWAPI::Broodwar->getFrameCount() - unit->getLastCommandFrame() > 16)
		{
			return true;
		}

		// this will be false if we have not yet started animating (turning, moving, etc)
		if (data.startedAttackAnim == false)
		{
			//BWAPI::Broodwar->printf("Haven't attacked yet");
			return false;
		}
		else
		{
			int framesSinceAnim = BWAPI::Broodwar->getFrameCount() - data.startedAttackAnimTime;

			bool answer = framesSinceAnim >= Search::StarcraftData::getAttackFrames(unit->getType()).first;

			if (answer == false)
			{
				//BWAPI::Broodwar->printf("Anim thing");
			}

			// TODO: add logic for init/rpt move
			return answer;
		}
	}
	else if (lastCommand.type == BWAPI::UnitCommandTypes::Move)
	{
		if (!unit->isMoving())
		{
			return true;
		}
		else
		{
			return unit->getDistance(targetPosition) < 28;
		}
	}
	else
	{
		//BWAPI::Broodwar->printf("Last command unknown");
		return false;
	}
}


bool MicroSearchManager::canIssueStopCommand(BWAPI::Unit * unit)
{
	AnimData data;
	if (attackAnimData.find(unit) != attackAnimData.end())
	{
		data = attackAnimData[unit];
	}

	BWAPI::UnitCommand unitC = BWAPI::UnitCommand::attack(unit, unit, false);

	BWAPI::UnitCommand lastCommand = unit->getLastCommand();

	// if the last command on this unit was to attack, do checks to see if we will interrupt attacks
	if (lastCommand.type == BWAPI::UnitCommandTypes::Attack_Unit)
	{
		// this will be false if we have not yet started animating (turning, moving, etc)
		if (data.startedAttackAnim == false)
		{
			// here we will always be interrupting the attack so don't
			return false;
		}
		else
		{
			// frames since we started animating the attack
			int framesSinceAnim = BWAPI::Broodwar->getFrameCount() - data.startedAttackAnimTime;

			// return whether or not enough frames have passed for damage to have been done
			return framesSinceAnim >= Search::StarcraftData::getAttackFrames(unit->getType()).first;
		}
	}
	// otherwise, the last command was something else
	else
	{
		// only allow stop command if the last command was stop
		return unit->getGroundWeaponCooldown() > 2;
	}
}

void MicroSearchManager::drawUnitCooldown(BWAPI::Unit * unit)
{
	double totalCooldown = unit->getType().groundWeapon().damageCooldown();
	double remainingCooldown = unit->getGroundWeaponCooldown();
	double percCooldown = remainingCooldown / (totalCooldown+1);

	int w = 32;
	int h = 4;

	int cw = w - (int)(w * percCooldown);
	int ch = h;

	int x1 = unit->getPosition().x() - w/2;
	int y1 = unit->getPosition().y() - 16;

	BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + w, y1 + h, BWAPI::Colors::Grey, true);
	BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + cw, y1 + ch, BWAPI::Colors::Red, true);
}

void MicroSearchManager::drawUnitHP(BWAPI::Unit * unit)
{
	double totalHP = unit->getType().maxHitPoints() + unit->getType().maxShields();
	double currentHP = unit->getHitPoints() + unit->getShields();
	double percHP = currentHP / (totalHP+1);

	int w = 32;
	int h = 4;

	int cw = (int)(w * percHP);
	int ch = h;

	int x1 = unit->getPosition().x() - w/2;
	int y1 = unit->getPosition().y() - 12;

	BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + w, y1 + h, BWAPI::Colors::Grey, true);
	BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + cw, y1 + ch, BWAPI::Colors::Green, true);
}

void MicroSearchManager::doUnitMove(MicroSearch::GameState & currentState, MicroSearch::Unit & unit, MicroSearch::Move & move)
{
	IDType enemyPlayer = (unit.player() + 1) % 2;

	BWAPI::Unit * u = BWAPI::Broodwar->getUnit(unit.ID());

	if (move._moveType == MicroSearch::MoveTypes::ATTACK)
	{
		BWAPI::Broodwar->drawTextMap(unit.x()+5, unit.y()+5, "A");
		MicroSearch::Unit & enemyUnit(currentState.getUnit(enemyPlayer, move._moveIndex));
		
		smartAttackUnit(u, BWAPI::Broodwar->getUnit(enemyUnit.ID()));
		//BWAPI::Broodwar->getUnit(unit.ID())->attack(BWAPI::Broodwar->getUnit(enemyUnit.ID()));
	}
	else if (move._moveType == MicroSearch::MoveTypes::MOVE)
	{
		BWAPI::Broodwar->drawTextMap(unit.x()+5, unit.y()+5, "M");
		MicroSearch::Position pos(Search::Constants::Move_Dir[move._moveIndex][0], Search::Constants::Move_Dir[move._moveIndex][1]);

		// tell the unit to move
		MicroSearch::Position dest(unit.x() + (pos.x() * 4*Search::Constants::Move_Distance), unit.y() + (pos.y() * 4*Search::Constants::Move_Distance));
		
		smartMove(u, BWAPI::Position(dest.x(), dest.y()));
		//BWAPI::Broodwar->getUnit(unit.ID())->move(BWAPI::Position(dest.x(), dest.y()));
	}
	else if (move._moveType == MicroSearch::MoveTypes::RELOAD)
	{
		BWAPI::Broodwar->drawTextMap(unit.x()+5, unit.y()+5, "W");
		//BWAPI::Broodwar->printf("Wait Action Returned");
		//if (canIssueStopCommand(u))
		{
			attackAnimData[u] = AnimData(false, BWAPI::Broodwar->getFrameCount(), BWAPI::UnitCommandTypes::Stop, BWAPI::Broodwar->getFrameCount());
			//u->stop();
			//BWAPI::Broodwar->printf("Stop Issued");
		}
	}
}

void MicroSearchManager::drawCurrentState()
{
	MicroSearch::GameState currentState = extractGameState();

	int currentFrame = BWAPI::Broodwar->getFrameCount();

	// draw our units
	for (size_t u(0); u<currentState.numUnits(Search::Players::Player_One); ++u)
	{
		MicroSearch::Unit & unit = currentState.getUnit(Search::Players::Player_One, u);
		BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Green);
		BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), unit.range(), BWAPI::Colors::Grey);

		std::pair<int, int> cooldown = getUnitCooldown(BWAPI::Broodwar->getUnit(unit.ID()), unit);

		BWAPI::Broodwar->drawTextMap(unit.x(), unit.y(), "%d (%d %d %d)", unit.ID(), cooldown.first-currentFrame, cooldown.second-currentFrame, BWAPI::Broodwar->getUnit(unit.ID())->getGroundWeaponCooldown());
	}

	// TODO: Check why zealots aren't being given commands

	// draw their units
	for (size_t u(0); u<currentState.numUnits(Search::Players::Player_Two); ++u)
	{
		MicroSearch::Unit & unit = currentState.getUnit(Search::Players::Player_Two, u);
		drawUnitHP(BWAPI::Broodwar->getUnit(unit.ID()));
		//BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Red);
	}

	// draw our moves if we are the player to move
	const IDType whoCanMove = currentState.whoCanMove();
	if ((whoCanMove == Search::Players::Player_One) || (whoCanMove == Search::Players::Player_Both))
	{
		// get the best move tuple from the current state
		MoveTuple bestTuple = getMoveTuple(currentState, Search::PlayerModels::AlphaBeta);
	
		// extract all of the moves possible from the current state
		MicroSearch::MoveArray moves;
		currentState.generateMoves(moves, Search::Players::Player_One);

		// draw the best move for each unit
		for (size_t u(0); u<moves.numUnitsInTuple(); ++u)
		{
			// the move for the first unit to move
			MicroSearch::Move move = moves.getTupleMove(bestTuple, u);

			// the unit for which the move is to be performed
			MicroSearch::Unit & unit = currentState.getUnit(Search::Players::Player_One, move.unit());
			BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Red);
		
			// draw the move this unit should do
			drawUnitMove(currentState, unit, move);
			drawUnitCooldown(BWAPI::Broodwar->getUnit(unit.ID()));
			drawUnitHP(BWAPI::Broodwar->getUnit(unit.ID()));
			doUnitMove(currentState, unit, move);
		}
	}
}

MicroSearch::MicroSearchParameters MicroSearchManager::getSearchParameters() const
{
	MicroSearch::MicroSearchParameters params;
	params.setMaxPlayer(Search::Players::Player_One);
	params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
	params.setPlayerToMoveMethod(Search::PlayerToMove::Alternate);
	//params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::AttackClosest, true);
	params.setEvalMethod(Search::EvaluationMethods::ModelSimulation);
	params.setMaxDepth(50);
	params.setTimeLimit(35);

	return params;
}

void MicroSearchManager::drawSearchResults(int x, int y)
{
	
/*	BWAPI::Broodwar->drawBoxScreen(x-5, y-15, x+125, y+55, BWAPI::Colors::Black, true);

	BWAPI::Broodwar->drawTextScreen(x, y-13,	"\x07Search Information");
				
	BWAPI::Broodwar->drawTextScreen(x, y,		"\x04 Total Time");
	BWAPI::Broodwar->drawTextScreen(x+75, y,	"%.3lf ms", previousResults.timeElapsed);

	BWAPI::Broodwar->drawTextScreen(x, y+10,	"\x04 Nodes");
	BWAPI::Broodwar->drawTextScreen(x+75, y+10,	"%d", (int)previousResults.nodesExpanded);

	BWAPI::Broodwar->drawTextScreen(x, y+20,	"\x04 Max Depth");
	BWAPI::Broodwar->drawTextScreen(x+75, y+40,	"%d", (int)previousResults.maxDepthReached);

	BWAPI::Broodwar->drawTextScreen(x, y+30,	"\x04 Best MoveTuple");
	BWAPI::Broodwar->drawTextScreen(x+75, y+40,	"%d", (int)previousResults.bestMoveTuple);
	*/
}

const IDType MicroSearchManager::getPlayerID(BWAPI::Player * player) const
{
	return (player == BWAPI::Broodwar->self()) ? Search::Players::Player_One : Search::Players::Player_Two;
}

const bool MicroSearchManager::isCombatUnit(BWAPI::Unit * unit) const
{
	assert(unit != NULL);

	// no workers or buildings allowed
	if (unit && unit->getType().isWorker() || unit->getType().isBuilding())
	{
		return false;
	}

	// check for various types of combat units
	if (unit->getType().canAttack() || 
		unit->getType() == BWAPI::UnitTypes::Terran_Medic ||
		unit->getType() == BWAPI::UnitTypes::Protoss_High_Templar ||
		unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
	{
		return true;
	}
		
	return false;
}

MicroSearch::Player * MicroSearchManager::getMicroSearchPlayer(const IDType & playerModel, const IDType & player) const
{
	MicroSearch::Player * p = NULL;

	if (playerModel == Search::PlayerModels::AlphaBeta)
	{
		MicroSearch::Player_AlphaBeta * abp = new MicroSearch::Player_AlphaBeta(player);
		abp->setParameters(getSearchParameters());
		p = abp;
	}
	else if (playerModel == Search::PlayerModels::AttackClosest)
	{
		p = new MicroSearch::Player_AttackClosest(player);
	}
	else if (playerModel == Search::PlayerModels::Kiter)
	{
		p = new MicroSearch::Player_Kiter(player);
	}
	else 
	{
		p = new MicroSearch::Player_NOK_AttackDPS(player);
	}

	return p;
}