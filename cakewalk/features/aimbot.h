#pragma once
// used: winapi includes, singleton
#include "../common.h"
// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: angle
#include "../sdk/datatypes/qangle.h"
// used: baseentity
#include "../sdk/entity.h"

#include "../utilities.h"

class CAimBot : public CSingleton<CAimBot>
{
public:
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);

private:
};
