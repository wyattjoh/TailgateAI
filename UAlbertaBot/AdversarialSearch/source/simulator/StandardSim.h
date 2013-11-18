#ifndef STANDARD_SIM_H
#define STANDARD_SIM_H

#include "Simulator.h"
#include "SpecProps.h"

namespace sim
{
	class PlayerWeapon
	{
		const Player *		player;
		BWAPI::WeaponType	type;
	public:
							PlayerWeapon(const Player * player, BWAPI::WeaponType type) : player(player), type(type) {}

		int					GetDamageBase() const										{ return WeaponProps::Get(type).GetDamageBase(*player); }
		int					GetDamageMultiplier(BWAPI::UnitSizeType targetSize) const	{ return WeaponProps::Get(type).GetDamageMultiplier(targetSize); }
		int					GetCooldown() const											{ return WeaponProps::Get(type).GetCooldown(*player); }
		int					GetMaxRange() const											{ return WeaponProps::Get(type).GetMaxRange(*player); }
	};

	class PlayerUnit
	{
		const Player *		player;
		BWAPI::UnitType		type;
	public:
							PlayerUnit() {}
							PlayerUnit(const Player & player, BWAPI::UnitType type) : player(&player), type(type) {}

		BWAPI::UnitType		GetType() const			{ return type; }
		BWAPI::UnitSizeType	GetSize() const			{ return type.size(); }
		int					GetSpeed() const		{ return UnitProps::Get(type).GetSpeed(*player); }
		int					GetSight() const		{ return UnitProps::Get(type).GetSight(*player); }
		int					GetMaxHitPoints() const	{ return type.maxHitPoints(); }
		int					GetMaxShields() const	{ return type.maxShields(); }
		int					GetMaxEnergy() const	{ return UnitProps::Get(type).GetMaxEnergy(*player); }
		int					GetCapacity() const		{ return UnitProps::Get(type).GetCapacity(*player); }
		int					GetArmor() const		{ return UnitProps::Get(type).GetArmor(*player); }
		int					GetShieldArmor() const	{ return player->GetUpgradeLevel(BWAPI::UpgradeTypes::Protoss_Plasma_Shields); }

		bool				CanAttack(const PlayerUnit & target) const { return (target.type.isFlyer() ? type.airWeapon() : type.groundWeapon()) != BWAPI::WeaponTypes::None; }
		PlayerWeapon		GetWeapon(const PlayerUnit & target) const { return PlayerWeapon(player, target.type.isFlyer() ? type.airWeapon() : type.groundWeapon()); }

		static void			Init();
	};

	class StandardForce;

	class StandardUnit
	{
		const StandardForce *	force;

		PlayerUnit				unit;
		int						position;
		int						hpQuarters;
		int						shields;
		ubyte					energy;
		ubyte					hangarCount;
		ubyte					cooldown;
		const BWAPI::Unit *		originalUnit;

		bool					active;
		StandardUnit *			target;
		const BWAPI::Unit *		firstTarget;
	public:
		StandardUnit() {}
		StandardUnit(const StandardForce * force, const Player & player, const Unit & unit, float vx, float vy) :
			force(force),
			unit(player, unit.GetType()),
			position(static_cast<int>((unit.GetPosition().x() << pixelShift) * vx + (unit.GetPosition().y() << pixelShift) * vy)),
			hpQuarters(unit.GetHitPoints() << 2),
			shields(unit.GetShields()),
			energy(unit.GetEnergy()),
			hangarCount(unit.GetHangarCount()),
			cooldown(unit.GetCooldown()),
			originalUnit(unit.GetOriginalUnit()),
			active(true),
			target(0),
			firstTarget(0)
		{

		}

		bool				IsAlive() const { return hpQuarters > 0; }
		bool				IsActive() const { return active; }
		bool				IsFlyer() const { return GetType().isFlyer(); }
		BWAPI::UnitType		GetType() const { return unit.GetType(); }
		int					GetPosition() const { return position; }
		int					GetVisibleHitPoints() const	{ return hpQuarters >> 2; }
		int					GetShields() const { return shields; }
		int					GetEnergy() const { return energy; }
		int					GetHangarCount() const { return hangarCount; }
		int					GetCooldown() const { return cooldown; }
		int					GetOverkill() const	{ return hpQuarters < 0 ? -hpQuarters : 0; }
		bool				CanAttack(const StandardUnit & target) const { return unit.CanAttack(target.unit); }
		const BWAPI::Unit *	GetOriginalUnit() const { return originalUnit; }
		const BWAPI::Unit *	GetFirstTarget() const { return firstTarget; }

		void				SetPosition(int position) { this->position = position; }
		void				Advance(const Params & params, size_t timestep, StandardForce & enemyStandardForce);
		void				Attack(StandardUnit & target);
		void				ReceiveAttack(const PlayerWeapon & weapon);
	};
	typedef std::vector<StandardUnit> StandardUnits;

	class StandardForce
	{
		StandardUnits				units;
		std::vector<StandardUnit*>	alive;
		std::vector<StandardUnit*>	active;
		bool						hasDetection;
	public:
		StandardForce() : hasDetection(false) {}

		void Reset(const Force & force, float vx, float vy)
		{
			units.clear();
			foreach(const Unit & unit, force.units)
			{
				units.push_back(StandardUnit(this, force.player, unit, vx, vy));
			}

			ComputeActive();
		}

		void UpdateForce(Force & force) const;

		void GetFirstTargets(ISimulator::TargetPairs & targetPairs) const;

		void ComputeActive()
		{
			alive.clear();
			for(StandardUnits::iterator it(units.begin()), end(units.end()); it!=end; ++it)
			{
				StandardUnit & unit(*it);
				if(unit.IsAlive())
				{
					alive.push_back(&unit);
				}
			}

			hasDetection = false;
			active.clear();
			foreach(StandardUnit * unit, alive)
			{
				if(unit->IsActive())
				{
					active.push_back(unit);
				}

				if(unit->GetType().isDetector())
				{
					hasDetection = true;
				}
			}
		}

		bool HasUnits() const
		{
			return !alive.empty();
		}

		bool HasTargets() const
		{
			return !active.empty();
		}

		bool HasDetection() const
		{
			return hasDetection;
		}

		int GetMinCooldown() const
		{
			int minCooldown(0x7fffffff);
			foreach(const StandardUnit * unit, active)
			{
				minCooldown = std::min(minCooldown, unit->GetCooldown());
			}
			return minCooldown;
		}

		void SetPosition(int position)
		{
			for(StandardUnits::iterator it(units.begin()), end(units.end()); it!=end; ++it)
			{
				it->SetPosition(position);
			}
		}

		StandardUnit * SelectTarget(const Params & params, const StandardUnit & aggressor, bool aggHasDetection)
		{
			StandardUnit * aliveTarget(0);
			foreach(StandardUnit * target, alive)		// Try to return highest priority active unit
			{
				if(!target->IsAlive())					// If target has just died, skip
				{
					continue;
				}

				if(!aggHasDetection)						// If we lack detection and the target is cloaked, skip
				{
					if(target->GetType().hasPermanentCloak())
					{
						continue;
					}

					// TODO: Terran cloak, zerg burrow
				}

				if(aggressor.CanAttack(*target))		// If this unit is targetable
				{
					// If the target is a threat or if we are forcing the ordering
					if(target->IsActive() || params.forceOrdering)
					{
						return target;					// Return this unit as our new target
					}

					// Otherwise, if we currently have no target
					else if(!aliveTarget)
					{
						aliveTarget = target;			// Store this target in case we find no active target
					}
				}
			}
			return aliveTarget;							// Return highest priority living unit
		}

		void Advance(const Params & params, size_t timestep, StandardForce & enemyStandardForce)
		{
			foreach(StandardUnit * unit, active)
			{
				unit->Advance(params, timestep, enemyStandardForce);
			}
		}
	};

	class StandardSimulator : public ISimulator
	{
		State					state;
		StandardForce			forceA;
		StandardForce			forceB;

		void					Advance(const Params & params, size_t timestep);
		void					UpdateState();
		bool					IsFinished(const Params & params) const;
	public:
		const State &			GetState() const { return state; }
		void					GetFirstTargets(ISimulator::TargetPairs & targetPairs) const;

		void					Reset(const State & state);
		void					Simulate(size_t time, const Params & params);
		void					Resolve(const Params & params);
	};
};

#endif