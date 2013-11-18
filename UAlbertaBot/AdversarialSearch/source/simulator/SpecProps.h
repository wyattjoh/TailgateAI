#ifndef SPEC_PROPS_H
#define SPEC_PROPS_H

#include "Simulator.h"

namespace sim
{
	extern const int				pixelShift;
	extern const int				damageMultipliers[7][6];

	class WeaponProps
	{
		static WeaponProps			props[256];

		BWAPI::WeaponType			type;

		BWAPI::UpgradeType			rangeUpgrade;
		BWAPI::UpgradeType			speedUpgrade;

		int							cooldown[2];
		int							maxRange[2];

		void						SetRangeUpgrade(BWAPI::UpgradeType upgrade, int maxRange);
		void						SetSpeedUpgrade(BWAPI::UpgradeType upgrade, int cooldown);
		void						SetType(BWAPI::WeaponType type);
	public:
									WeaponProps();

		int							GetDamageBase(const Player & player) const { return type.damageAmount() + player.GetUpgradeLevel(type.upgradeType()) * type.damageFactor(); }
		int							GetDamageMultiplier(BWAPI::UnitSizeType targetSize) const { return damageMultipliers[type.damageType().getID()][targetSize.getID()]; }
		int							GetCooldown(const Player & player) const { return cooldown[player.GetUpgradeLevel(speedUpgrade)]; }
		int							GetMaxRange(const Player & player) const { return maxRange[player.GetUpgradeLevel(rangeUpgrade)]; }

		static const WeaponProps &	Get(BWAPI::WeaponType type) { return props[type.getID()]; }
		static void					Init();
	};

	class UnitProps
	{
		static UnitProps			props[256];

		BWAPI::UnitType				type;

		BWAPI::UpgradeType			capacityUpgrade;
		BWAPI::UpgradeType			extraArmorUpgrade;
		BWAPI::UpgradeType			maxEnergyUpgrade;
		BWAPI::UpgradeType			sightUpgrade;
		BWAPI::UpgradeType			speedUpgrade;

		int							capacity[2];
		int							extraArmor[2];
		int							maxEnergy[2];
		int							sightRange[2];
		int							speed[2];

		void						SetCapacityUpgrade(BWAPI::UpgradeType upgrade, int capacity0, int capacity1);
		void						SetEnergyUpgrade(BWAPI::UpgradeType upgrade);
		void						SetExtraArmorUpgrade(BWAPI::UpgradeType upgrade, int amount);
		void						SetSightUpgrade(BWAPI::UpgradeType upgrade, int range);
		void						SetSpeedUpgrade(BWAPI::UpgradeType upgrade, double rate);
		void						SetType(BWAPI::UnitType type);
	public:
									UnitProps();

		int							GetArmor(const Player & player) const		{ return type.armor() + player.GetUpgradeLevel(type.armorUpgrade()) + extraArmor[player.GetUpgradeLevel(extraArmorUpgrade)]; }
		int							GetCapacity(const Player & player) const	{ return capacity[player.GetUpgradeLevel(capacityUpgrade)]; }
		int							GetMaxEnergy(const Player & player) const	{ return maxEnergy[player.GetUpgradeLevel(maxEnergyUpgrade)]; }
		int							GetSight(const Player & player) const		{ return sightRange[player.GetUpgradeLevel(sightUpgrade)]; }
		int							GetSpeed(const Player & player) const		{ return speed[player.GetUpgradeLevel(speedUpgrade)]; }

		const WeaponProps &			GetGroundWeapon() const						{ return WeaponProps::Get(type.groundWeapon()); }
		const WeaponProps &			GetAirWeapon() const						{ return WeaponProps::Get(type.airWeapon()); }

		static const UnitProps &	Get(BWAPI::UnitType type)					{ return props[type.getID()]; }
		static void					Init();
	};
}

#endif