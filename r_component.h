#pragma once

#include "game/client/gameclient.h"

#define LOCAL g_Config.m_ClDummy
#define LOCAL_ID m_pClient->m_aLocalIds[LOCAL]

#define DUMMY !g_Config.m_ClDummy
#define DUMMY_ID m_pClient->m_aLocalIds[DUMMY]

class RComponent
{
public:
	RComponent(CGameClient *pClient) :
		m_pClient(pClient) {}
	virtual ~RComponent() = default;

protected:
	// CGameClient pointer
	CGameClient *m_pClient;

	// Game classes pointers
	[[nodiscard]] CGameClient *GameClient() const { return m_pClient; }
	[[nodiscard]] CControls *Controls() const { return &m_pClient->m_Controls; }
	[[nodiscard]] IConsole *Console() const { return m_pClient->Console(); }
	[[nodiscard]] IInput *Input() const { return m_pClient->Input(); }
	[[nodiscard]] CCollision *Collision() const { return m_pClient->Collision(); }
	[[nodiscard]] IClient *Client() const { return m_pClient->Client(); }
	[[nodiscard]] IGraphics *Graphics() const { return m_pClient->Graphics(); }
	[[nodiscard]] CGameWorld *GameWorld() const { return &m_pClient->m_GameWorld; }
	[[nodiscard]] CGameWorld *PredGameWorld() const { return &m_pClient->m_PredictedWorld; }
	[[nodiscard]] CTuningParams *Tuning() const { return m_pClient->m_aTuning; }
	[[nodiscard]] IServerBrowser *ServerBrowser() const { return m_pClient->ServerBrowser(); }
};