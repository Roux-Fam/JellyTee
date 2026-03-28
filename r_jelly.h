#ifndef GAME_CLIENT_ROUX_R_JELLY_H
#define GAME_CLIENT_ROUX_R_JELLY_H

#include <base/vmath.h>

#include <memory>

class CGameClient;

struct JellyTee
{
	vec2 m_BodyScale = vec2(1.f, 1.f);
	vec2 m_FeetScale = vec2(1.f, 1.f);
	float m_BodyAngle = 0.f;
	float m_FeetAngle = 0.f;
};

class CRJelly
{
public:
	explicit CRJelly(CGameClient *pClient);

	void Reset();
	JellyTee GetDeform(int ClientId, vec2 PrevVel, vec2 Vel, vec2 LookDir, bool InAir, bool WantOtherDir, float DeltaTime);

private:
	struct CState
	{
		vec2 m_Deform = vec2(0.f, 0.f);
		vec2 m_DeformVelocity = vec2(0.f, 0.f);
		vec2 m_PrevInputVel = vec2(0.f, 0.f);
		float m_Compression = 0.f;
		float m_CompressionVelocity = 0.f;
		int m_ClientId = -1;
		bool m_Initialized = false;
	};

	CGameClient *m_pClient = nullptr;
	CState m_State;

	bool IsEnabledFor(int ClientId) const;
};

extern std::unique_ptr<CRJelly> rJelly;

#endif
