# Jelly Tee Integration for DDNet


## 1. Добавить конфиги

Нужные переменные:

```cpp
MACRO_CONFIG_INT(RouxTeeJelly, roux_tee_jelly, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Roux jelly tee effect")
MACRO_CONFIG_INT(RouxTeeJellyStrength, roux_tee_jelly_strength, 100, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Strength of jelly tee deformation")
MACRO_CONFIG_INT(RouxTeeJellyDuration, roux_tee_jelly_duration, 100, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Duration of jelly tee deformation")
```

## 2. Подключить модуль в gameclient

в:
- [src/game/client/gameclient.cpp](/src/game/client/gameclient.cpp)

Нужно:

1. Подключить include:

```cpp
#include <game/client/Roux/r_jelly.h>
```

2. где-то в```cpp
void CGameClient::OnConsoleInit()```:
вставь
```cpp
rJelly = std::make_unique<CRJelly>(this);
```

3. где-то в ```cpp
void CGameClient::OnReset()
```:
вставь 

```cpp
if(rJelly)
	rJelly->Reset();
```

## 3. Расширить RenderTee (самое сложное)

Изменения находятся в:
- [src/game/client/render.h](/src/game/client/render.h)
- [src/game/client/render.cpp](/src/game/client/render.cpp)

Это изменение никак не повлияет на другой код

в
- [src/game/client/render.h](/src/game/client/render.h)

замените определение
```cpp
void RenderTee6(...)
void RenderTee7(...)
```
на
```cpp
void RenderTee6(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha = 1.0f, vec2 BodyScale = vec2(1.0f, 1.0f), vec2 FeetScale = vec2(1.0f, 1.0f), float BodyAngle = 0.0f, float FeetAngle = 0.0f) const;
	void RenderTee7(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha = 1.0f, vec2 BodyScale = vec2(1.0f, 1.0f), vec2 FeetScale = vec2(1.0f, 1.0f), float BodyAngle = 0.0f, float FeetAngle = 0.0f) const;
```
замените определение 
```cpp
void RenderTee(...)
```
на 
```cpp
void RenderTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha = 1.0f, vec2 BodyScale = vec2(1.0f, 1.0f), vec2 FeetScale = vec2(1.0f, 1.0f), float BodyAngle = 0.0f, float FeetAngle = 0.0f) const;
```
в
- [src/game/client/render.cpp](/src/game/client/render.cpp)

в 
```cpp
void CRenderTools::RenderTee7(...)
```
замените 
```cpp
IGraphics::CQuadItem BodyItem(BodyPos.x, BodyPos.y, BaseSize, BaseSize);
```
на
```cpp
IGraphics::CQuadItem BodyItem(BodyPos.x, BodyPos.y, BaseSize * BodyScale.x, BaseSize * BodyScale.y);
```
замените 

```cpp
IGraphics::CQuadItem BotItem(BodyPos.x + (2.f / 3.f) * AnimScale, BodyPos.y + (-16 + 2.f / 3.f) * AnimScale, BaseSize, BaseSize);
```
на
```cpp
IGraphics::CQuadItem BotItem(BodyPos.x + (2.f / 3.f) * AnimScale, BodyPos.y + (-16 + 2.f / 3.f) * AnimScale, BaseSize * BodyScale.x, BaseSize * BodyScale.y);
```
замените (7 раз)

```cpp
Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
```
на
```cpp
Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2 + BodyAngle);
```
после 

```cpp
const CAnimKeyframe *pFoot = Filling ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();
```
вставьте это:
```cpp
         float w = (BaseSize / 2.1f) * FeetScale.x;
			float h = (BaseSize / 2.1f) * FeetScale.y;
```
в
```cpp 
void CRenderTools::RenderTee6(...)
```
замените 
```cpp
	Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
```
на 
```cpp
   Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2 + BodyAngle);
```
замените 
```cpp
            float BodyScale;
				GetRenderTeeBodyScale(BaseSize, BodyScale);
```
на 
```cpp
            float RenderBodyScale;
				GetRenderTeeBodyScale(BaseSize, RenderBodyScale);
```

замените 
```cpp
Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, OutLine, BodyPos.x, BodyPos.y, BodyScale, BodyScale);
```
на 
```cpp
Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, OutLine, BodyPos.x, BodyPos.y, RenderBodyScale * BodyScale.x, RenderBodyScale * BodyScale.y);
```

замените 
```cpp
					float EyeScale = BaseSize * 0.40f;
					float h = Emote == EMOTE_BLINK ? BaseSize * 0.15f : EyeScale;
					float EyeSeparation = (0.075f - 0.010f * absolute(Direction.x)) * BaseSize;
					vec2 Offset = vec2(Direction.x * 0.125f, -0.05f + Direction.y * 0.10f) * BaseSize;
```
на 
```cpp
					float EyeScale = BaseSize * 0.40f * BodyScale.x;
					float h = (Emote == EMOTE_BLINK ? BaseSize * 0.15f : BaseSize * 0.40f) * BodyScale.y;
					float EyeSeparation = (0.075f - 0.010f * absolute(Direction.x)) * BaseSize * BodyScale.x;
					vec2 Offset = vec2(Direction.x * 0.125f * BodyScale.x, (-0.05f + Direction.y * 0.10f) * BodyScale.y) * BaseSize;

```

замените 
```cpp
			float w = BaseSize;
			float h = BaseSize / 2;
```
на 
```cpp
			float w = BaseSize * FeetScale.x;
			float h = (BaseSize / 2) * FeetScale.y;
```

замените 
```cpp
			Graphics()->QuadsSetRotation(pFoot->m_Angle * pi * 2);
```
на 
```cpp
			Graphics()->QuadsSetRotation(pFoot->m_Angle * pi * 2 + FeetAngle);
```
замените код 
```cpp
void CRenderTools::RenderTee(...)
```
на 
```cpp
void CRenderTools::RenderTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha, vec2 BodyScale, vec2 FeetScale, float BodyAngle, float FeetAngle) const
{
	if(pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_BODY).IsValid())
		RenderTee7(pAnim, pInfo, Emote, Dir, Pos, Alpha, BodyScale, FeetScale, BodyAngle, FeetAngle);
	else
		RenderTee6(pAnim, pInfo, Emote, Dir, Pos, Alpha, BodyScale, FeetScale, BodyAngle, FeetAngle);

	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);
	Graphics()->QuadsSetRotation(0);
}
```



## 4. Подключить в players.cpp

Файл:
- [src/game/client/components/players.cpp](/src/game/client/components/players.cpp)

Что нужно сделать:

1. Подключить:

```cpp
#include <game/client/Roux/r_jelly.h>
```

2. Ввести изменения:
перед
```cpp
vec2 Vel = mix(vec2(Prev.m_VelX / 256.0f, Prev.m_VelY / 256.0f), vec2(Player.m_VelX / 256.0f, Player.m_VelY / 256.0f), Intra);
```
вставь:

```cpp
vec2 PrevVel = vec2(Prev.m_VelX / 256.0f, Prev.m_VelY / 256.0f);
```
после
```cpp
bool Inactive = ClientId >= 0 && (GameClient()->m_aClients[ClientId].m_Afk || GameClient()->m_aClients[ClientId].m_Paused);
```
вставь:

```cpp
const JellyTee jellyDeform = rJelly ? rJelly->GetDeform(ClientId, PrevVel, Vel, Direction, InAir, WantOtherDir, Client()->RenderFrameTime()) : JellyTee();
```

3. Передать деформу в `RenderTee(...)`:

```cpp
RenderTools()->RenderTee(&State, &RenderInfo, Player.m_Emote, Direction, Position, Alpha, jellyDeform.m_BodyScale, jellyDeform.m_FeetScale, jellyDeform.m_BodyAngle, jellyDeform.m_FeetAngle);
```

То же самое желательно передавать и в рендер тени / ghost, если хочешь, чтобы деформация совпадала визуально
