#include "Simulator.h"

using namespace sim;


////////////
// Player //
////////////

Player::Player()
{
	Reset();
}

Player::Player(const BWAPI::Player & player) 
{ 
	Capture(player); 
}

void Player::Reset()
{
	for(int i(0); i<NUM_UPGRADES; ++i)
	{
		upgradeLevel[i] = 0;
	}

	for(int i(0); i<NUM_TECHS; ++i)
	{
		hasResearched[i] = false;
	}
}

void Player::SetUpgradeLevel(BWAPI::UpgradeType upgrade, int level)
{
	assert(upgrade != BWAPI::UpgradeTypes::None);
	assert(upgrade != BWAPI::UpgradeTypes::Unknown);
	assert(level >= 0 && level <= upgrade.maxRepeats());
	upgradeLevel[upgrade.getID()] = level;
}

void Player::SetResearched(BWAPI::TechType tech, bool researched)
{
	assert(tech != BWAPI::TechTypes::None); 
	assert(tech != BWAPI::TechTypes::Unknown); 
	hasResearched[tech.getID()] = researched;
}

void Player::Capture(const BWAPI::Player & player)
{
	for(int i(0); i<NUM_UPGRADES; ++i)
	{
		upgradeLevel[i] = player.getUpgradeLevel(i);
	}

	for(int i(0); i<NUM_TECHS; ++i)
	{
		hasResearched[i] = player.hasResearched(i);
	}
}



//////////
// Unit //
//////////

Unit::Unit(BWAPI::UnitType type)
{
	Reset(type);
}

Unit::Unit(BWAPI::UnitType type, const BWAPI::Position & position)
{
	Reset(type);
	SetPosition(position);
}

Unit::Unit(const BWAPI::Unit & unit)
{
	Capture(unit);
}

void Unit::Reset(BWAPI::UnitType type)
{
	this->type			= type.getID();
	this->position		= BWAPI::Position(0,0);
	this->hitPoints		= type.maxHitPoints();
	this->shields		= type.maxShields();
	this->energy		= type.maxEnergy();
	this->hangarCount	= 0;
	this->cooldown		= 0;

	this->originalUnit	= 0;
}

void Unit::SetPosition(const BWAPI::Position & position)
{
	this->position = position;
}

void Unit::SetHitPoints(int hitPoints)
{
	assert(hitPoints > 0);
	this->hitPoints = hitPoints;
}

void Unit::SetShields(int shields)
{
	assert(shields >= 0);
	this->shields = shields;
}

void Unit::SetEnergy(int energy)
{
	assert(energy >= 0);
	this->energy = energy;
}

void Unit::SetHangarCount(int hangarCount)
{
	assert(hangarCount >= 0);
	this->hangarCount = hangarCount;
}

void Unit::SetCooldown(int cooldown)
{
	assert(cooldown >= 0);
	this->cooldown = cooldown;
}

void Unit::SetOriginalUnit(const BWAPI::Unit * unit)
{
	this->originalUnit = unit;
}

void Unit::Capture(const BWAPI::Unit & unit)
{
	type			= unit.getType().getID();
	position		= unit.getPosition();
	hitPoints		= unit.getHitPoints();
	shields			= unit.getShields();
	energy			= unit.getEnergy();

	if(unit.getType() == BWAPI::UnitTypes::Terran_Vulture)
	{
		hangarCount	= unit.getSpiderMineCount();
	}
	else if(unit.getType() == BWAPI::UnitTypes::Protoss_Carrier)
	{
		hangarCount	= unit.getInterceptorCount();
	}
	else if(unit.getType() == BWAPI::UnitTypes::Protoss_Reaver)
	{
		hangarCount	= unit.getScarabCount();
	}
	else
	{
		hangarCount	= 0;
	}

	cooldown		= std::max(std::max(unit.getGroundWeaponCooldown(), unit.getAirWeaponCooldown()), unit.getSpellCooldown());

	originalUnit	= &unit;
}



///////////
// Force //
///////////

template<typename FORWARD_ITERATOR>
void CaptureUnits(Units & units, size_t count, FORWARD_ITERATOR first, FORWARD_ITERATOR last)
{
	size_t index(units.size());
	units.resize(index + count);
	for(; first != last; ++first)
	{
		units[index++].Capture(**first);
	}
}

void Force::AddUnits(BWAPI::UnitType type, size_t count)
{
	units.insert(units.end(), count, type);
}

void Force::AddUnits(BWAPI::UnitType type, const BWAPI::Position & position, size_t count)
{
	units.insert(units.end(), count, Unit(type,position));
}

void Force::AddUnits(const std::vector<const BWAPI::Unit *> & newUnits)
{
	CaptureUnits(units, newUnits.size(), newUnits.begin(), newUnits.end());
}

void Force::AddUnits(const std::vector<BWAPI::Unit *> & newUnits)
{
	CaptureUnits(units, newUnits.size(), newUnits.begin(), newUnits.end());
}

void Force::AddUnits(const std::set<const BWAPI::Unit *> & newUnits)
{
	CaptureUnits(units, newUnits.size(), newUnits.begin(), newUnits.end());
}

void Force::AddUnits(const std::set<BWAPI::Unit *> & newUnits)
{
	CaptureUnits(units, newUnits.size(), newUnits.begin(), newUnits.end());
}