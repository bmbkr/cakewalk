#pragma once
// used: winapi includes
#include "../common.h"

// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: listener event function
#include "../sdk/interfaces/igameeventmanager.h"
// used: baseentity
#include "../sdk/entity.h"

class CMiscellaneous : public CSingleton<CMiscellaneous>
{
public:
	// Get
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);
	void Event(IGameEvent* pEvent, const FNV1A_t uNameHash);
	void MovementCorrection(CUserCmd* pCmd, const QAngle& angOldViewPoint) const;

private:
	bool bTryAutoBuy = false;

	void AutoBuy();
	void AutoHealthShot();
	void AutoReload(CUserCmd* pCmd, CBaseEntity* pLocal);
};
