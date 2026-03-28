#include "r_jelly.h"

#include <base/math.h>

#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <cmath>

std::unique_ptr<CRJelly> rJelly;

namespace
{
	constexpr float minDeltaTime = 1.f / 240.f;
	constexpr float maxDeltaTime = 1.f / 20.f;

	vec2 NormalizeOr(vec2 value, vec2 fallback)
	{
		const float valueLength = length(value);
		if(valueLength > 0.0001f)
			return value / valueLength;

		const float fallbackLength = length(fallback);
		if(fallbackLength > 0.0001f)
			return fallback / fallbackLength;

		return vec2(0.0f, -1.0f);
	}

	float DurationScale()
	{
		const float duration = g_Config.m_RouxTeeJellyDuration / 100.f;
		return std::pow(std::clamp(duration, 0.2f, 5.f), 1.65f);
	}

	void ClampDeform(JellyTee &deform)
	{
		deform.m_BodyScale.x = std::clamp(deform.m_BodyScale.x, 0.94f, 1.22f);
		deform.m_BodyScale.y = std::clamp(deform.m_BodyScale.y, 0.74f, 1.22f);
		deform.m_FeetScale.x = std::clamp(deform.m_FeetScale.x, 0.95f, 1.15f);
		deform.m_FeetScale.y = std::clamp(deform.m_FeetScale.y, 0.70f, 1.10f);
	}
}

CRJelly::CRJelly(CGameClient *pClient) :
	m_pClient(pClient)
{
}

void CRJelly::Reset()
{
	m_State = CState();
}

bool CRJelly::IsEnabledFor(int ClientId) const
{
	return m_pClient != nullptr && g_Config.m_RouxTeeJelly && ClientId >= 0 && ClientId == m_pClient->m_aLocalIds[g_Config.m_ClDummy];
}

JellyTee CRJelly::GetDeform(int ClientId, vec2 PrevVel, vec2 Vel, vec2 LookDir, bool InAir, bool WantOtherDir, float DeltaTime)
{
	JellyTee deform;
	const bool disabled = !g_Config.m_RouxEnabled || !g_Config.m_RouxTeeJelly || g_Config.m_RouxTeeJellyStrength <= 0;

	if(disabled && m_State.m_Initialized)
	{
		Reset();
		return deform;
	}

	if(!IsEnabledFor(ClientId) || disabled)
		return deform;

	if(m_State.m_ClientId != ClientId)
		Reset();

	m_State.m_ClientId = ClientId;
	DeltaTime = std::clamp(DeltaTime, minDeltaTime, maxDeltaTime);

	if(!m_State.m_Initialized)
	{
		m_State.m_PrevInputVel = Vel;
		m_State.m_Initialized = true;
	}

	const float strength = g_Config.m_RouxTeeJellyStrength / 100.0f;
	const float durationScale = DurationScale();

	//собираем импульсы движения
	const vec2 previousVel = m_State.m_PrevInputVel;
	const vec2 deltaVel = Vel - previousVel;
	const bool directionFlipX = (Vel.x > 0.0f && previousVel.x < 0.0f) || (Vel.x < 0.f && previousVel.x > 0.f);
	const float landingImpact = (InAir ? 0.0f : std::clamp((PrevVel.y - Vel.y) / 13.0f, 0.f, 1.8f)) * strength;
	const float stopImpulse = std::clamp((length(previousVel) - length(Vel)) / 4.8f, 0.0f, 1.3f) * strength;
	const float turnImpulse = std::clamp(absolute(deltaVel.x) / 5.8f, 0.f, directionFlipX ? 1.2f : 0.8f) * strength;
	const float sideImpulse = std::clamp(absolute(deltaVel.x) / 5.2f, 0.f, directionFlipX ? 1.15f : 0.75f) * strength;

	vec2 motionBasis = NormalizeOr(Vel, vec2(LookDir.x, -0.25f));
	vec2 targetDeform(
		std::clamp(-deltaVel.x / 6.0f, -0.9f, 0.9f) * strength,
		std::clamp(-deltaVel.y / 10.0f, -1.0f, 1.0f) * strength);
	targetDeform.x += std::clamp(-deltaVel.x / 5.0f, -0.5f, 0.5f) * sideImpulse;
	targetDeform += vec2(std::clamp(-Vel.x / 22.0f, -0.28f, 0.28f), 0.f) * stopImpulse;
	if(directionFlipX || WantOtherDir)
		targetDeform.x += std::clamp(-Vel.x / 17.0f, -0.28f, 0.28f) * strength;

	//тянем текущую форму тела к целевой деформации от движения
	const float deformSpring = 6.0f / durationScale;
	const float deformDamping = 1.9f / maximum(durationScale, 0.35f);
	m_State.m_DeformVelocity += (targetDeform - m_State.m_Deform) * deformSpring * DeltaTime;
	m_State.m_DeformVelocity *= 1.0f / (1.0f + deformDamping * DeltaTime);
	m_State.m_Deform += m_State.m_DeformVelocity * DeltaTime;

	//отвечает за более резкие импульсы/остановка/приземление и тп
	const float targetCompression = landingImpact * 1.25f + stopImpulse * 0.55f + turnImpulse * 0.30f - std::clamp(-Vel.y / 24.0f, 0.0f, 0.25f);
	const float compressionSpring = 4.2f / durationScale;
	const float compressionDamping = 1.7f / maximum(durationScale, 0.35f);
	m_State.m_CompressionVelocity += (targetCompression - m_State.m_Compression) * compressionSpring * DeltaTime;
	m_State.m_CompressionVelocity *= 1.0f / (1.0f + compressionDamping * DeltaTime);
	m_State.m_Compression += m_State.m_CompressionVelocity * DeltaTime;

	//обрабатываем результаты
	const vec2 deformDirection = NormalizeOr(m_State.m_Deform, motionBasis);
	const float deformAmount = std::clamp(length(m_State.m_Deform), 0.0f, 1.10f);
	const float horizontalStretch = absolute(deformDirection.x) * deformAmount;
	const float verticalStretch = absolute(deformDirection.y) * deformAmount;
	const float landingSquash = std::clamp(m_State.m_Compression, 0.0f, 1.2f);
	const float airStretch = std::clamp(-m_State.m_Compression, 0.0f, 0.5f);

	deform.m_BodyScale.x += horizontalStretch * 0.14f + landingSquash * 0.24f + verticalStretch * 0.03f;
	deform.m_BodyScale.y += verticalStretch * 0.15f + airStretch * 0.22f - horizontalStretch * 0.05f - landingSquash * 0.34f;

	deform.m_FeetScale.x += landingSquash * 0.15f + horizontalStretch * 0.03f;
	deform.m_FeetScale.y += airStretch * 0.05f - landingSquash * 0.24f - horizontalStretch * 0.01f;

	deform.m_BodyAngle = std::clamp(-m_State.m_Deform.x * 0.12f - m_State.m_DeformVelocity.x * 0.008f + deformDirection.x * verticalStretch * 0.02f, -0.12f, 0.12f);
	deform.m_FeetAngle = std::clamp(-deform.m_BodyAngle * 0.30f + m_State.m_Deform.x * 0.025f, -0.06f, 0.06f);
	ClampDeform(deform);

	m_State.m_PrevInputVel = Vel;
	return deform;
}
