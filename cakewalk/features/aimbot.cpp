#include "aimbot.h"

#include "../core/variables.h"
#include "../global.h"
#include "../core/interfaces.h"
#include "../features/autowall.h"
#include "../utilities/math.h"
#include "../utilities/logging.h"
#include "../sdk/convar.h"

void CAimBot::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	DampenSpread(pCmd);
	BeginFixMove(pCmd);
	if (C::Get<bool>(Vars.bAimbotEnabled))
		ExecuteRageBot(pCmd, pLocal);
	EndFixMove(pCmd);
}

void CAimBot::ExecuteRageBot(CUserCmd* pCmd, CBaseEntity* pLocal) {
	m_bIsAimbotting = false;
	int iBestTarget = GetBestTarget(pCmd, pLocal);

	if (iBestTarget == -1)
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

	if (pWeaponData == nullptr || !pWeaponData->IsGun() || pWeaponData->iBullets <= 0)
		return;

	CBaseEntity* pBestTarget = I::ClientEntityList->GetClientEntity(iBestTarget)->GetBaseEntity();

	Vector vHitbox;
	switch (C::Get<int>(Vars.nAimbotHitscan))
	{
	case 0:
		vHitbox = pBestTarget->GetHitboxPosition(C::Get<int>(Vars.nAimbotHitbox)).value();
		break;
	case 1:
		BestAimPointAll(pBestTarget, vHitbox);
		break;
	case 2:
		BestAimPointHitbox(pBestTarget, vHitbox);
		break;
	case 3:
		BestAimPoint(pBestTarget, vHitbox);
		break;
	case 4:
		CustomHitScans(pBestTarget, vHitbox);
		break;
	default:
		return;
	}

	if (!vHitbox.IsZero()) {
		QAngle qTarget = M::CalcAngle(pLocal->GetEyePosition(), vHitbox);

		QAngle qSwitchBack = pCmd->angViewPoint;
		qTarget -= qSwitchBack; // possible bug
		qTarget.Normalize();

		if (C::Get<bool>(Vars.bAimbotAutoScope))
			if (!pLocal->IsScoped() && pWeapon->HasScope())
				pCmd->iButtons |= IN_SECOND_ATTACK;

		if ((C::Get<bool>(Vars.bAimbotAutoScope) && pWeapon->HasScope() && pLocal->IsScoped()) || !pWeapon->HasScope() || !C::Get<bool>(Vars.bAimbotAutoScope)) {
			// TODO: AUTO STOP HERE
			if (m_bIsStopped && CanHit(pWeapon)) {
				if (pCmd->iButtons & IN_ATTACK || C::Get<bool>(Vars.bAimbotAutoShoot)) {
					pCmd->angViewPoint = qTarget;
					pCmd->angViewPoint -= pLocal->GetPunch() * 2.0f;
					pCmd->angViewPoint.Normalize();
					if (!C::Get<bool>(Vars.bAimbotSilent))
						I::Engine->SetViewAngles(pCmd->angViewPoint);
					m_bIsAimbotting = true;
				}
				m_iTarget = iBestTarget;
				if (C::Get<bool>(bAimbotAutoShoot)) {
					if (pWeaponData->bFullAuto && !C::Get<bool>(Vars.bAimbotOneTap)) {
						pCmd->iButtons |= IN_ATTACK;
					}
					else {
						static bool bDoShoot = false;
						if (bDoShoot)
							pCmd->iButtons |= IN_ATTACK;
						else
							pCmd->iButtons &= ~IN_ATTACK;
						bDoShoot = !bDoShoot;
					}
				}
			}
		}
		else if(pLocal->IsScoped() && pWeapon->HasScope()){
			pCmd->iButtons |= IN_SECOND_ATTACK;
		}
	}
}

void CAimBot::DampenSpread(CUserCmd* pCmd) {
	CBaseCombatWeapon* pWeapon = G::pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

	if (pWeaponData == nullptr || !pWeaponData->IsGun() || pWeaponData->iBullets <= 0)
		return;

	float flMaxSpeed = pWeaponData->flMaxSpeed * 0.34f;
	float flVelocity = G::pLocal->GetVelocity().Length();
	if (flVelocity > flMaxSpeed) {
		m_bIsStopped = false;
		if (m_bIsAimbotting) {
			pCmd->flForwardMove = flMaxSpeed - 20.0f;
			pCmd->flSideMove = 0.0f;
		}
	}
	else {
		m_bIsStopped = true;
	}
}

void CAimBot::BeginFixMove(CUserCmd* pCmd) {
	m_qOldAngles = pCmd->angViewPoint;
	m_flOldForwardMove = pCmd->flForwardMove;
	m_flOldSideMove = pCmd->flSideMove;
}

void CAimBot::EndFixMove(CUserCmd* pCmd) {
	float flDeltaYaw = pCmd->angViewPoint.y - m_qOldAngles.y;
	float flOne, flTwo;

	if (m_qOldAngles.y < 0.0f)
		flOne = 360.0f + m_qOldAngles.y;
	else
		flOne = m_qOldAngles.y;

	if (pCmd->angViewPoint.y < 0.0f)
		flTwo = 360.0f + pCmd->angViewPoint.y;
	else
		flTwo = pCmd->angViewPoint.y;

	if (flTwo < flOne)
		flDeltaYaw = fabsf(flTwo - flOne);
	else
		flDeltaYaw = 360.0f - fabsf(flOne - flTwo);

	flDeltaYaw = 360.0f - flDeltaYaw;

	pCmd->flForwardMove = cosf(M_DEG2RAD(flDeltaYaw)) * m_flOldForwardMove + cosf(M_DEG2RAD(flDeltaYaw + 90.0f)) * m_flOldSideMove;
	pCmd->flSideMove = sinf(M_DEG2RAD(flDeltaYaw)) * m_flOldForwardMove + sinf(M_DEG2RAD(flDeltaYaw + 90.0f)) * m_flOldSideMove;
}

bool CAimBot::CanHit(CBaseCombatWeapon* pWeapon) {
	return true; // ADD HITCHANCE STUFF <3
}

#define CHECKFINDSTART Vector vTempHitbox; \
bool bDidFind = false; \
int nHitscanType = C::Get<int>(Vars.nAimbotHitscan); \
switch (nHitscanType) \
{ \
case 0: \
	bDidFind = true; \
	break; \
case 1: \
	bDidFind = BestAimPointAll(pEntity, vTempHitbox); \
	break; \
case 2: \
	bDidFind = BestAimPointHitbox(pEntity, vTempHitbox); \
	break; \
case 3: \
	bDidFind = BestAimPoint(pEntity, vTempHitbox); \
	break; \
case 4: \
	bDidFind = CustonHitscans(pEntity, vTempHitbox); \
	break; \
default: \
	break; \
} \
if (bDidFind) { \
	iBestTarget = i; \

#define CHECKFINDEND \
}

int CAimBot::GetBestTarget(CUserCmd* pCmd, CBaseEntity* pLocal) {
	float flFov = 360.0f;
	float flDistance = 8192.0f;
	float flThreatLevel = 0.0f;
	float flHealth = 101.0f;
	float flDamage = 0.0f;
	int iBestTarget = -1;
	for (size_t i = 1; i < I::Engine->GetMaxClients(); i++) {
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(i)->GetBaseEntity();
		if (!IsValidEntity(pEntity))
			continue;

		Vector vHitboxPos = pEntity->GetHitboxPosition(C::Get<int>(Vars.nAimbotHitbox)).value();
		if(m_veciBones[pEntity->GetIndex()] >= 0)
			vHitboxPos = pEntity->GetBonePosition(m_veciBones[pEntity->GetIndex()]).value();

		int iTargetingMode = C::Get<int>(Vars.nAimbotTargetMode);
		switch (iTargetingMode)
		{
		case TARGETMODE_FOV: {
			QAngle qTempTargetAngles = M::CalcAngle(pLocal->GetEyePosition(), vHitboxPos);
			float flTempFov = M::GetFOV(pCmd->angViewPoint, qTempTargetAngles);
			if (flTempFov < flFov) {
				CHECKFINDSTART
					flFov = flTempFov;
				CHECKFINDEND
			}
			break;
		}
		case TARGETMODE_HEALTH: {
			float flTempHealth = pEntity->GetHealth();
			if (flTempHealth < flHealth) {
				CHECKFINDSTART
					flHealth = flTempHealth;
				CHECKFINDEND
			}
			break;
		}
		case TARGETMODE_DISTANCE: {
			QAngle qTempTargetAngles = M::CalcAngle(pLocal->GetEyePosition(), vHitboxPos);
			float flTempFov = M::GetFOV(pCmd->angViewPoint, qTempTargetAngles);
			float flTempDistance = pLocal->GetOrigin().DistTo(pEntity->GetOrigin());
			if (flTempDistance < flDistance && flTempFov < flFov) {
				CHECKFINDSTART
					flFov = flTempFov;
					flDistance = flTempDistance;
				CHECKFINDEND
			}
			break;
		}
		case TARGETMODE_FIRSTVALID: {
			if (!iBestTarget) {
				CHECKFINDSTART
				CHECKFINDEND
			}
			break;
		}
		case TARGETMODE_NEXTSHOT: {
			// dunno
			break;
		}
		case TARGETMODE_THREAT: {
			float flTempDistance = pLocal->GetOrigin().DistTo(pEntity->GetOrigin());
			float flTempHealth = pEntity->GetHealth();
			float flTempThreat = flTempHealth / flTempDistance;
			// TODO: Check aimNearPlayer here!!
			if (pEntity->IsTargetingLocal(pLocal))
				flTempThreat += 100.0f;
			if (flTempThreat > flThreatLevel) {
				CHECKFINDSTART
					flThreatLevel = flTempThreat;
				CHECKFINDEND
			}
			break;
		}
		case TARGETMODE_VISIBLEBONE: {
			FireBulletData_t fbData;
			CAutoWall::Get().GetDamage(G::pLocal, vHitboxPos, fbData);
			float flTempDamage = fbData.flCurrentDamage;
			if (C::Get<float>(Vars.flAimbotAutowallMinDamage) <= flTempDamage) {
				iBestTarget = i;
			}
			break;
		}
		case TARGETMODE_DAMAGE: {
			FireBulletData_t fbData;
			CAutoWall::Get().GetDamage(G::pLocal, vHitboxPos, fbData);
			float flTempDamage = fbData.flCurrentDamage;
			if (flTempDamage > flDamage) {
				CHECKFINDSTART
					flDamage = flTempDamage;
				CHECKFINDEND
			}
			break;
		}
		default:
			break;
		}
	}
	return iBestTarget;
}

bool CAimBot::IsValidEntity(CBaseEntity* pEntity) {
	if (pEntity == nullptr)
		return false;

	if (pEntity == G::pLocal)
		return false;

	if (pEntity->IsDormant())
		return false;

	if (!pEntity->IsPlayer())
		return false;

	if (pEntity->GetTeam() == G::pLocal->GetTeam())
		return false;

	if (pEntity->GetHealth() <= 0 || !pEntity->IsAlive())
		return false;

	if (pEntity->HasImmunity())
		return false;

	Vector vTemp = pEntity->GetBonePosition(C::Get<int>(Vars.nAimbotHitbox)).value();
	if (m_veciBones[pEntity->GetIndex()] >= 0)
		vTemp = pEntity->GetBonePosition(m_veciBones[pEntity->GetIndex()]).value();

	int nHitscanType = C::Get<int>(Vars.nAimbotHitscan);
	if (nHitscanType == 0) {
		FireBulletData_t fbData;
		CAutoWall::Get().GetDamage(G::pLocal, vTemp, fbData);
		float flDamage = fbData.flCurrentDamage;
		if (C::Get<float>(Vars.flAimbotAutowallMinDamage) <= flDamage)
			return true;
	}
	else {
		return true;
	}
}

std::vector<int> SetupHitboxes() {
	std::vector<int> veciTemp{};
	veciTemp.push_back(HITBOX_HEAD);
	veciTemp.push_back(HITBOX_NECK);
	veciTemp.push_back(HITBOX_PELVIS);
	veciTemp.push_back(HITBOX_THORAX);
	veciTemp.push_back(HITBOX_CHEST);
	veciTemp.push_back(HITBOX_UPPER_CHEST);
	veciTemp.push_back(HITBOX_RIGHT_THIGH);
	veciTemp.push_back(HITBOX_LEFT_THIGH);
	veciTemp.push_back(HITBOX_RIGHT_CALF);
	veciTemp.push_back(HITBOX_LEFT_CALF);
	veciTemp.push_back(HITBOX_RIGHT_FOOT);
	veciTemp.push_back(HITBOX_LEFT_FOOT);
	veciTemp.push_back(HITBOX_RIGHT_HAND);
	veciTemp.push_back(HITBOX_LEFT_HAND);
	veciTemp.push_back(HITBOX_RIGHT_UPPER_ARM);
	veciTemp.push_back(HITBOX_RIGHT_FOREARM);
	veciTemp.push_back(HITBOX_LEFT_UPPER_ARM);
	veciTemp.push_back(HITBOX_LEFT_UPPER_ARM);
}

bool CAimBot::GetBestPoint(CBaseEntity* pTarget, Hitbox* pHitbox, BestPoint* pBestPoint) {
	Vector vCenter = pHitbox->points[0];

	if (pHitbox->hitbox == HITBOX_HEAD) {
		auto high = ((pHitbox->points[3] + pHitbox->points[5]) * .5f);

		auto pitch = pTarget->GetEyeAngles().x;
		if ((pitch > 0.f) && (pitch < 88.9f))
		{
			auto height = (((high - pHitbox->points[0]) / 3) * 4);
			auto new_height = (pHitbox->points[0] + (height * (pitch / 89.f)));

			pHitbox->points[0] = new_height;
			pBestPoint->flags |= FL_HIGH;
		}
		else if ((pitch < 292.5f) && (pitch > 270.9f))
		{
			pHitbox->points[0] -= Vector(0.f, 0.f, 1.f);
			pBestPoint->flags |= FL_LOW;
		}
	}

	for (size_t i = 0; i < 9; ++i) {

		FireBulletData_t fbData;
		CAutoWall::Get().GetDamage(G::pLocal, pHitbox->points[i], fbData);
		float flDamage = fbData.flCurrentDamage;

		if (pBestPoint->dmg < flDamage) {
			pBestPoint->dmg = flDamage;
			pBestPoint->point = pHitbox->points[i];
			pBestPoint->index = i;
		}
	}
}

bool CAimBot::BestAimPointAll(CBaseEntity* pEntity, Vector& vTempHitbox) {
	BestPoint bpAimPoint;

	for (size_t i = 0; i < 7; i++) {
		if (m_vecbDisableHead[pEntity->GetIndex()])
			if (i == HITBOX_HEAD)
				continue;
		Hitbox hBox()
	}
};

bool CAimBot::BestAimPointHitbox(CBaseEntity* pEntity, Vector& vTempHitbox) {

};

bool CAimBot::BestAimPoint(CBaseEntity* pEntity, Vector& vTempHitbox) {

};

bool CAimBot::CustomHitscans(CBaseEntity* pEntity, Vector& vTempHitbox) {
	BestPoint bpAimPoint;
	std::vector<int> veciHitboxes = SetupHitboxes();
	for (const int iHitbox : veciHitboxes) {
		if (m_vecbDisableHead[pEntity->GetIndex()])
			if (iHitbox == HITBOX_HEAD)
				continue;
		Hitbox hBox(iHitbox);
		GetHitbox(pEntity, &hBox);
		if (!CustomMultipoint(pEntity, &hBox, &bpAimPoint))
			continue;
	}

	if (bpAimPoint.dmg > C::Get<float>(Vars.flAimbotAutowallMinDamage)) {
		vTempHitbox = bpAimPoint.point;
		return true;
	}
	return false;
};
