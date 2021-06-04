// used: random_device, mt19937, uniform_int_distribution
#include <random>

#include "misc.h"
// used: global variables
#include "../global.h"
// used: cheat variables
#include "../core/variables.h"
// used: convar interface
#include "../core/interfaces.h"
#include "../features/aimbot.h"
// used: angle-vector calculations
#include "../utilities/math.h"

void CMiscellaneous::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	if (this->bTryAutoBuy && pLocal->IsAlive()) {
		this->bTryAutoBuy = false;
		this->AutoBuy();
	}

	this->AutoHealthShot();
	this->AutoReload(pCmd, pLocal);
}

void CMiscellaneous::Event(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!I::Engine->IsInGame())
		return;

	if (uNameHash == FNV1A::HashConst(XorStr("player_death"))) {
		if (I::Engine->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")) != G::pLocal->GetIndex()))
			return;

		this->bTryAutoBuy = true;
	}
}

void CMiscellaneous::AutoBuy()
{
	if (C::Get<bool>(Vars.bMiscAutoBuy)) {
		I::Engine->ExecuteClientCmd(C::Get<std::string>(Vars.szMiscAutoBuyString).c_str());
	}
}

void CMiscellaneous::AutoHealthShot() {
	//if(G::pLocal->)
}

void CMiscellaneous::AutoReload(CUserCmd* pCmd, CBaseEntity* pLocal) {
	if (!C::Get<bool>(Vars.bMiscAutoReload))
		return;

	if (CAimBot::Get().GetTimeSinceLastShot() < C::Get<int>(Vars.iMiscAutoReloadTimeSinceShot))
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

	if (pWeaponData == nullptr || !pWeaponData->IsGun())
		return;

	if (pWeapon->IsReloading())
		return;

	float flRatio = (float)pWeapon->GetAmmo() / pWeaponData->iMaxClip1;

	if (flRatio > C::Get<float>(Vars.flMiscAutoReloadAmmoThreshold))
		return;

	pCmd->iButtons |= IN_RELOAD;
}

void CMiscellaneous::MovementCorrection(CUserCmd* pCmd, const QAngle& angOldViewPoint) const
{
	static CConVar* cl_forwardspeed = I::ConVar->FindVar(XorStr("cl_forwardspeed"));

	if (cl_forwardspeed == nullptr)
		return;

	static CConVar* cl_sidespeed = I::ConVar->FindVar(XorStr("cl_sidespeed"));

	if (cl_sidespeed == nullptr)
		return;

	static CConVar* cl_upspeed = I::ConVar->FindVar(XorStr("cl_upspeed"));

	if (cl_upspeed == nullptr)
		return;

	// get max speed limits by convars
	const float flMaxForwardSpeed = cl_forwardspeed->GetFloat();
	const float flMaxSideSpeed = cl_sidespeed->GetFloat();
	const float flMaxUpSpeed = cl_upspeed->GetFloat();

	Vector vecForward = { }, vecRight = { }, vecUp = { };
	M::AngleVectors(angOldViewPoint, &vecForward, &vecRight, &vecUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.f;

	vecForward.NormalizeInPlace();
	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	Vector vecOldForward = { }, vecOldRight = { }, vecOldUp = { };
	M::AngleVectors(pCmd->angViewPoint, &vecOldForward, &vecOldRight, &vecOldUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.f;

	vecOldForward.NormalizeInPlace();
	vecOldRight.NormalizeInPlace();
	vecOldUp.NormalizeInPlace();

	const float flPitchForward = vecForward.x * pCmd->flForwardMove;
	const float flYawForward = vecForward.y * pCmd->flForwardMove;
	const float flPitchSide = vecRight.x * pCmd->flSideMove;
	const float flYawSide = vecRight.y * pCmd->flSideMove;
	const float flRollUp = vecUp.z * pCmd->flUpMove;

	// solve corrected movement
	const float x = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;
	const float y = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;
	const float z = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;

	// clamp and apply corrected movement
	pCmd->flForwardMove = std::clamp(x, -flMaxForwardSpeed, flMaxForwardSpeed);
	pCmd->flSideMove = std::clamp(y, -flMaxSideSpeed, flMaxSideSpeed);
	pCmd->flUpMove = std::clamp(z, -flMaxUpSpeed, flMaxUpSpeed);
}

