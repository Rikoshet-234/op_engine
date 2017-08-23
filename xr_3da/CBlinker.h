#ifndef CBlinker_h
#define CBlinker_h
#pragma once

#include "iinputreceiver.h"
#include "effector.h"

class ENGINE_API CBlinker : public CEffectorCam,	public IInputReceiver
{
private:
	Fvector		m_HPB;
	Fvector		m_Position;
	Fmatrix		m_Camera;
	u32			m_Stage;

	float		m_fSpeed0;
	float		m_fSpeed1;
	float		m_fSpeed2;
	float		m_fSpeed3;
	float		m_fAngSpeed0;
	float		m_fAngSpeed1;
	float		m_fAngSpeed2;
	float		m_fAngSpeed3;

	Fvector		m_vT;
	Fvector		m_vR;
	Fvector		m_vVelocity;
	Fvector		m_vAngularVelocity;
public:
	CBlinker(float life_time = 60 * 60 * 1000);
	virtual ~CBlinker();

	void IR_OnKeyboardPress(int dik) override;
	void IR_OnKeyboardHold(int dik) override;
	void IR_OnMouseMove(int dx, int dy) override;
	void IR_OnMouseHold(int btn) override;

	virtual BOOL Overlapped()  { return FALSE; }
	virtual BOOL Process(Fvector &p, Fvector &d, Fvector &n, float& fFov, float& fFar, float& fAspect);
};
#endif
