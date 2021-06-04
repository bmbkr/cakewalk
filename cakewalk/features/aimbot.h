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

struct BestPoint
{
	BestPoint(void)
	{
		hitbox = -1;
		point = Vector(0, 0, 0);
		index = -1;
		dmg = -1;
		flags = 0;
	}

	explicit BestPoint(int newHitBox)
	{
		hitbox = newHitBox;
		point = Vector(0, 0, 0);
		index = -1;
		dmg = -1;
		flags = 0;
	}

	Vector point;
	int  index;
	int  dmg;
	int  flags;
	int  hitbox;
};

struct Hitbox
{
	Hitbox(void)
	{
		hitbox = -1;
	}

	Hitbox(int newHitBox)
	{
		hitbox = newHitBox;
	}

	int  hitbox;
	Vector points[9];
};

class CAimBot : public CSingleton<CAimBot>
{
public:
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);

private:
	bool m_bIsStopped;
	int m_iTarget;
	bool m_bIsAimbotting;
	QAngle m_qOldAngles;
	float m_flOldForwardMove;
	float m_flOldSideMove;
	std::vector<int> m_veciBones{ 64 };
	std::vector<bool> m_vecbDisableHead{ 64 };

	void ExecuteRageBot(CUserCmd* pCmd, CBaseEntity* pLocal);
	void DampenSpread(CUserCmd* pCmd);
	void BeginFixMove(CUserCmd* pCmd);
	void EndFixMove(CUserCmd* pCmd);
	bool CanHit(CBaseCombatWeapon* pWeapon);
	int GetBestTarget(CUserCmd* pCmd, CBaseEntity* pLocal);
	bool IsValidEntity(CBaseEntity* pEntity);
	std::vector<int> SetupHitboxes();

	bool GetBestPoint(CBaseEntity* pTarget, Hitbox* pHitbox, BestPoint* pBestPoint);
	bool BestAimPointAll(CBaseEntity* pEntity, Vector& vTempHitbox);
	bool BestAimPointHitbox(CBaseEntity* pEntity, Vector& vTempHitbox);
	bool BestAimPoint(CBaseEntity* pEntity, Vector& vTempHitbox);
	bool CustomHitscans(CBaseEntity* pEntity, Vector& vTempHitbox);
};
