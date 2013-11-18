
#include "StandardSim.h"

using namespace sim;

ISimulator * sim::CreateStandardSimulator()
{
	static bool init(true);
	if(init)
	{
		WeaponProps::Init();
		UnitProps::Init();
		init = false;
	}

	return new StandardSimulator;
}

void StandardSimulator::Advance(const Params & params, size_t timestep)
{
	state.frame += timestep;

	forceA.Advance(params, timestep, forceB);
	forceB.Advance(params, timestep, forceA);

	forceA.ComputeActive();
	forceB.ComputeActive();
}

void StandardSimulator::UpdateState()
{
	forceA.UpdateForce(state.a);
	forceB.UpdateForce(state.b);
}

void StandardSimulator::Reset(const State & state) 
{ 
	BWAPI::Position centroidA(0,0);
	foreach(const sim::Unit & unit, state.a.units)
	{
		centroidA += unit.GetPosition();
	}

	BWAPI::Position centroidB(0,0);
	foreach(const sim::Unit & unit, state.b.units)
	{
		centroidB += unit.GetPosition();
	}

	// Determine vector between unit centroids
	const float scaleA(1.0f / state.a.units.size());
	const float scaleB(1.0f / state.b.units.size());
	const float dx(centroidB.x() * scaleB - centroidA.x() * scaleA);
	const float dy(centroidB.y() * scaleB - centroidA.y() * scaleA);

	// Scale to unit length to create the 1D axis for simulating the battle
	const float len(sqrtf(dx*dx + dy*dy));
	const float vx(dx/len); 
	const float vy(dy/len);

	forceA.Reset(state.a, vx, vy);
	forceB.Reset(state.b, vx, vy);

	forceA.ComputeActive();
	forceB.ComputeActive();
}

bool StandardSimulator::IsFinished(const Params & params) const
{
	if(params.killAll)
	{
		// Finish if either force is completely wiped out, or if we have reached a stalemate
		return !forceA.HasUnits() || !forceB.HasUnits() || (!forceA.HasTargets() && !forceB.HasTargets());
	}
	else
	{
		// Finish if either force can no longer attack
		return !forceA.HasTargets() || !forceB.HasTargets();
	}
}

void StandardSimulator::Simulate(size_t time, const Params & params)
{
	while(true)
	{
		const size_t nextAction(std::min(forceA.GetMinCooldown(), forceB.GetMinCooldown()));
		if(IsFinished(params) || nextAction > time)
		{
			Advance(params, time);
			UpdateState();
			return;
		}

		Advance(params, nextAction);
		time -= nextAction;
	}
}

void StandardSimulator::Resolve(const Params & params)
{
	while(true)
	{
		const size_t nextAction(std::min(forceA.GetMinCooldown(), forceB.GetMinCooldown()));
		Advance(params, nextAction);

		if(IsFinished(params))
		{
			UpdateState();
			return;
		}
	}
}

#include <iostream>

void StandardUnit::Advance(const Params & params, size_t timestep, StandardForce & enemyStandardForce)
{
	cooldown -= timestep;
	while(cooldown == 0)
	{
		if(target && target->IsAlive())
		{
			Attack(*target);
		}
		else
		{
			target = enemyStandardForce.SelectTarget(params, *this, force->HasDetection());
			if(!target)
			{
				active = false;
				return;
			}
		}
	}
}

void StandardUnit::Attack(StandardUnit & target)
{
	assert(CanAttack(target));

	const PlayerWeapon weapon(unit.GetWeapon(target.unit));

	const int delta(target.position - position);
	if(abs(delta) > weapon.GetMaxRange())
	{
		position += unit.GetSpeed() * (delta > 0 ? 1 : -1);
		cooldown = 1;
	}
	else
	{
		target.ReceiveAttack(weapon);
		cooldown = weapon.GetCooldown();

		// Record the first target attacked by this unit
		if(!firstTarget)
		{
			firstTarget = target.originalUnit;
		}
	}
}

void StandardUnit::ReceiveAttack( const PlayerWeapon & weapon )
{
	int damage(weapon.GetDamageBase());

	// If unit has shields, deduct shield defense from damage dealt
	if(shields > 0)
	{
		damage = std::max(damage - unit.GetShieldArmor(), 1);
	}

	// Apply damage to shields, and retain damage that does not apply to shields
	shields -= damage;
	if(shields < 0)
	{
		damage	= -shields;
		shields = 0;

		// If we have any damage remaining after the shields, apply it
		if(damage > 0)
		{
			damage = std::max((damage-unit.GetArmor()) * weapon.GetDamageMultiplier(unit.GetSize()), 2);
			hpQuarters -= damage;
		}
	}
}

void StandardForce::UpdateForce(Force & force) const
{
	force.units.clear();
	foreach(const StandardUnit & simUnit, units)
	{
		if(simUnit.IsAlive())
		{
			Unit unit(simUnit.GetType());
			unit.SetHitPoints(std::max(simUnit.GetVisibleHitPoints(),0));
			unit.SetShields(std::max(simUnit.GetShields(),0));
			unit.SetEnergy(simUnit.GetEnergy());
			unit.SetHangarCount(simUnit.GetHangarCount());
			unit.SetCooldown(simUnit.GetCooldown());
			unit.SetOriginalUnit(simUnit.GetOriginalUnit());
			force.units.push_back(unit);
		}
	}
	force.isThreat = HasTargets();
}

void StandardForce::GetFirstTargets(ISimulator::TargetPairs & targetPairs) const
{
	foreach(const StandardUnit & unit, units)
	{
		targetPairs.push_back(std::make_pair(unit.GetOriginalUnit(), unit.GetFirstTarget()));
	}
}

void StandardSimulator::GetFirstTargets(ISimulator::TargetPairs & targetPairs) const
{
	forceA.GetFirstTargets(targetPairs);
	forceB.GetFirstTargets(targetPairs);
}
