#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include <BWAPI.h>
#include <cassert>
#include <cmath>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

namespace sim
{
	typedef unsigned char		ubyte;
	typedef unsigned short int	ushort;
	typedef unsigned int		uint;

	class Player
	{
		enum { NUM_UPGRADES	= 63, NUM_TECHS	= 47 };

		int						upgradeLevel[NUM_UPGRADES];
		bool					hasResearched[NUM_TECHS];
	public:
								Player();
								Player(const BWAPI::Player & player);

		int						GetUpgradeLevel(BWAPI::UpgradeType upgrade) const { return upgradeLevel[upgrade.getID()]; }
		bool					HasUpgrade(BWAPI::UpgradeType upgrade) const { return upgradeLevel[upgrade.getID()] > 0; }
		bool					HasResearched(BWAPI::TechType tech) const { return hasResearched[tech.getID()]; }

		void					Reset();
		void					SetUpgradeLevel(BWAPI::UpgradeType upgrade, int level);
		void					SetResearched(BWAPI::TechType tech, bool researched);
		void					Capture(const BWAPI::Player & player);
	};

	class Unit
	{
		ubyte					type;
		BWAPI::Position			position;
		ushort					hitPoints;
		ushort					shields;
		ubyte					energy;
		ubyte					hangarCount;
		ubyte					cooldown;

		const BWAPI::Unit *		originalUnit;
	public:
								Unit() {}
								Unit(BWAPI::UnitType type);
								Unit(BWAPI::UnitType type, const BWAPI::Position & position);
								Unit(const BWAPI::Unit & unit);

		BWAPI::UnitType			GetType() const { return type; }
		const BWAPI::Position &	GetPosition() const { return position; }
		int						GetHitPoints() const { return hitPoints; }
		int						GetShields() const { return shields; }
		int						GetEnergy() const { return energy; }
		int						GetHangarCount() const { return hangarCount; }
		int						GetCooldown() const { return cooldown; }
		const BWAPI::Unit *		GetOriginalUnit() const { return originalUnit; }

		void					Reset(BWAPI::UnitType type);
		void					SetPosition(const BWAPI::Position & position);
		void					SetHitPoints(int hitPoints);
		void					SetShields(int shields);
		void					SetEnergy(int energy);
		void					SetHangarCount(int hangarCount);
		void					SetCooldown(int cooldown);
		void					SetOriginalUnit(const BWAPI::Unit * unit);
		void					Capture(const BWAPI::Unit & unit);
	};
	typedef std::vector<Unit> Units;

	struct Force
	{
		Player					player;
		Units					units;
		bool					isThreat;

		void					AddUnits(BWAPI::UnitType type, size_t count);
		void					AddUnits(BWAPI::UnitType type, const BWAPI::Position & position, size_t count);
		void					AddUnits(const std::vector<const BWAPI::Unit *> & newUnits);
		void					AddUnits(const std::vector<BWAPI::Unit *> & newUnits);
		void					AddUnits(const std::set<const BWAPI::Unit *> & newUnits);
		void					AddUnits(const std::set<BWAPI::Unit *> & newUnits);

		/*bool					HasUnits() const { return !units.empty(); }
		bool					HasGroundUnits() const;
		bool					HasAirUnits() const;
		bool					HasCloakedUnits() const;

		bool					CanTargetGround() const;
		bool					CanTargetAir() const;
		bool					HasDetectors() const;*/
	};

	struct State
	{
		Force					a;
		Force					b;
		int						frame;

		State() : frame(0) {}
	};

	struct Params
	{
		bool killAll;		// If true, do not stop the simulation until one side is completely wiped out, or stalemate
		bool forceOrdering;	// If true, follow the exact ordering specified, regardless of whether units are active

		Params() : killAll(false), forceOrdering(false) {}
		Params(bool killAll, bool forceOrdering) : killAll(killAll), forceOrdering(forceOrdering) {}
	};

	struct ISimulator
	{
		typedef std::pair<const BWAPI::Unit *, const BWAPI::Unit *> TargetPair;
		typedef std::vector<TargetPair> TargetPairs;

		virtual const State &	GetState() const = 0;
		virtual void			GetFirstTargets(TargetPairs & targetPairs) const = 0;

		virtual void			Reset(const State & state) = 0;
		virtual void			Simulate(size_t time, const Params & params) = 0;
		virtual void			Resolve(const Params & params) = 0;
	};

	ISimulator * CreateStandardSimulator();
}

#endif