#pragma once
// used: define to add values to variables vector
#include "config.h"

enum EAimbotHitGroup : int
{
	BEST_DAMAGE = 0,
	HEAD,
	CHEST,
	STOMACH
};

enum EAimbotEnemySelection : int
{
	DISTANCE = 0,
	HIGHEST_HEALTH,
	LOWEST_HEALTH
};

struct Variables_t
{
	C_ADD_VARIABLE(bool, bAimbot, false);
	C_ADD_VARIABLE(bool, bAimbotSilent, false);
	C_ADD_VARIABLE(bool, bAimbotVisibleOnly, false);
	C_ADD_VARIABLE(bool, bAimbotChickens, false);
	C_ADD_VARIABLE(bool, bAimbotIgnoreWhileReloading, false);
	C_ADD_VARIABLE(bool, bAimbotAutoShoot, false);
	C_ADD_VARIABLE(bool, bAimbotNoRecoil, false);
	C_ADD_VARIABLE(bool, bAimbotStep, true);
	C_ADD_VARIABLE(int, iAimbotEnemySelection, EAimbotEnemySelection::DISTANCE);
	C_ADD_VARIABLE(int, iAimbotHitGroup, EAimbotHitGroup::BEST_DAMAGE);
	C_ADD_VARIABLE(int, iAimbotMinDamage, 10);

	C_ADD_VARIABLE(bool, bWalkbot, false);
	C_ADD_VARIABLE(bool, bWalkbotHumanize, false);
	C_ADD_VARIABLE(bool, bWalkbotVisualize, false);
	C_ADD_VARIABLE(bool, bWalkbotLookAtPoint, false);
	C_ADD_VARIABLE(bool, bWalkbotAutoOptimize, false);

	C_ADD_VARIABLE(bool, bMiscAutoBuy, false);
	C_ADD_VARIABLE(std::string, szMiscAutoBuyString, "");
	C_ADD_VARIABLE(bool, bMiscAutoReload, false);
	C_ADD_VARIABLE(float, flMiscAutoReloadAmmoThreshold, 0.1f);
	C_ADD_VARIABLE(int, iMiscAutoReloadTimeSinceShot, 5000);

	#pragma region variables_menu
	C_ADD_VARIABLE(int, iMenuKey, VK_INSERT);
	C_ADD_VARIABLE(int, iPanicKey, VK_END);
	#pragma endregion
};

inline Variables_t Vars;
