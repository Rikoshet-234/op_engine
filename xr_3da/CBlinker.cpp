#include "stdafx.h"
#include "CBlinker.h"

#include "igame_level.h"
#include "x_ray.h"

#include "gamefont.h"
#include "fDemoRecord.h"
#include "xr_ioconsole.h"
#include "xr_input.h"
#include "xr_object.h"
#include "render.h"
#include "CustomHUD.h"

CBlinker::CBlinker(float life_time) :CEffectorCam(cefDemo, life_time)
{
	
	IR_Capture();	
	m_Camera.invert(Device.mView);

	Fvector& dir = m_Camera.k;
	Fvector DYaw;	
	DYaw.set(dir.x, 0.f, dir.z); 
	DYaw.normalize_safe();
	if (DYaw.x<0)	
		m_HPB.x = acosf(DYaw.z);
	else
		m_HPB.x = 2 * PI - acosf(DYaw.z);

	dir.normalize_safe();
	m_HPB.y = asinf(dir.y);
	m_HPB.z = 0;

	m_Position.set(m_Camera.c);

	m_vVelocity.set(0, 0, 0);
	m_vAngularVelocity.set(0, 0, 0);

	m_vT.set(0, 0, 0);
	m_vR.set(0, 0, 0);
	
	m_fSpeed0 = pSettings->r_float("demo_record", "speed0");
	m_fSpeed1 = pSettings->r_float("demo_record", "speed1");
	m_fSpeed2 = pSettings->r_float("demo_record", "speed2");
	m_fSpeed3 = pSettings->r_float("demo_record", "speed3");
	m_fAngSpeed0 = pSettings->r_float("demo_record", "ang_speed0");
	m_fAngSpeed1 = pSettings->r_float("demo_record", "ang_speed1");
	m_fAngSpeed2 = pSettings->r_float("demo_record", "ang_speed2");
	m_fAngSpeed3 = pSettings->r_float("demo_record", "ang_speed3");
}

CBlinker::~CBlinker()
{
	IR_Release();
}

void CBlinker::IR_OnKeyboardPress(int dik)
{
	if(dik == DIK_GRAVE)
		Console->Show();
	if (dik == DIK_ESCAPE)	
		fLifeTime = -1;
	if (dik == DIK_RETURN)
	{
		if (g_pGameLevel->CurrentEntity())
		{
#ifndef NDEBUG
			g_pGameLevel->CurrentEntity()->ForceTransform(m_Camera);
#endif
			fLifeTime = -1;
		}
	}
}

void CBlinker::IR_OnKeyboardHold(int dik)
{
	switch (dik)
	{
	case DIK_A:
	case DIK_NUMPAD1:
	case DIK_LEFT:		m_vT.x -= 1.0f; break; // Slide Left
	case DIK_D:
	case DIK_NUMPAD3:
	case DIK_RIGHT:		m_vT.x += 1.0f; break; // Slide Right
	case DIK_S:			m_vT.y -= 1.0f; break; // Slide Down
	case DIK_W:			m_vT.y += 1.0f; break; // Slide Up
											   // rotate	
	case DIK_NUMPAD2:	m_vR.x -= 1.0f; break; // Pitch Down
	case DIK_NUMPAD8:	m_vR.x += 1.0f; break; // Pitch Up
	case DIK_E:
	case DIK_NUMPAD6:	m_vR.y += 1.0f; break; // Turn Left
	case DIK_Q:
	case DIK_NUMPAD4:	m_vR.y -= 1.0f; break; // Turn Right
	case DIK_NUMPAD9:	m_vR.z -= 2.0f; break; // Turn Right
	case DIK_NUMPAD7:	m_vR.z += 2.0f; break; // Turn Right
	}
}

void CBlinker::IR_OnMouseMove(int dx, int dy)
{
	float scale = .5f;//psMouseSens;
	if (dx || dy) 
	{
		m_vR.y += float(dx)*scale; // heading
		m_vR.x += ((psMouseInvert.test(1)) ? -1 : 1)*float(dy)*scale*(3.f / 4.f); // pitch
	}
}

void CBlinker::IR_OnMouseHold(int btn)
{
	switch (btn) 
	{
	case 0:			m_vT.z += 1.0f; break; // Move Backward
	case 1:			m_vT.z -= 1.0f; break; // Move Forward
	}
}

BOOL CBlinker::Process(Fvector& p, Fvector& d, Fvector& n, float& fFov, float& fFar, float& fAspect)
{
	if (psHUD_Flags.test(HUD_DRAW))
	{
		if ((Device.dwTimeGlobal / 750) % 3 != 0) {
			//				pApp->pFontSystem->SetSizeI	(0.02f);
			pApp->pFontSystem->SetColor(color_rgba(255, 255, 0, 255));
			pApp->pFontSystem->OutSet(0.2, 0.2);
			pApp->pFontSystem->SetAligment(CGameFont::alLeft);
			std::stringstream wss;
			pApp->pFontSystem->OutNext("Current position:");
			wss << L" X: " << m_Position.x;
			pApp->pFontSystem->OutNext(wss.str().c_str());
			wss << L" Y: " << m_Position.y;
			pApp->pFontSystem->OutNext(wss.str().c_str());
			wss << L" Z: " << m_Position.z;
			pApp->pFontSystem->OutNext(wss.str().c_str());
		}
	}
	m_vVelocity.lerp(m_vVelocity, m_vT, 0.3f);
	m_vAngularVelocity.lerp(m_vAngularVelocity, m_vR, 0.3f);

	float speed = m_fSpeed1, ang_speed = m_fAngSpeed1;
	if (Console->IR_GetKeyState(DIK_LSHIFT)) { speed = m_fSpeed0; ang_speed = m_fAngSpeed0; }
	else if (Console->IR_GetKeyState(DIK_LALT)) { speed = m_fSpeed2; ang_speed = m_fAngSpeed2; }
	else if (Console->IR_GetKeyState(DIK_LCONTROL)) { speed = m_fSpeed3; ang_speed = m_fAngSpeed3; }
	m_vT.mul(m_vVelocity, Device.fTimeDelta * speed);
	m_vR.mul(m_vAngularVelocity, Device.fTimeDelta * ang_speed);

	m_HPB.x -= m_vR.y;
	m_HPB.y -= m_vR.x;
	m_HPB.z += m_vR.z;

	// move
	Fvector vmove;

	vmove.set(m_Camera.k);
	vmove.normalize_safe();
	vmove.mul(m_vT.z);
	m_Position.add(vmove);

	vmove.set(m_Camera.i);
	vmove.normalize_safe();
	vmove.mul(m_vT.x);
	m_Position.add(vmove);

	vmove.set(m_Camera.j);
	vmove.normalize_safe();
	vmove.mul(m_vT.y);
	m_Position.add(vmove);

	m_Camera.setHPB(m_HPB.x, m_HPB.y, m_HPB.z);
	m_Camera.translate_over(m_Position);

	// update camera
	n.set(m_Camera.j);
	d.set(m_Camera.k);
	p.set(m_Camera.c);

	fLifeTime -= Device.fTimeDelta;

	m_vT.set(0, 0, 0);
	m_vR.set(0, 0, 0);
	return TRUE;
}
