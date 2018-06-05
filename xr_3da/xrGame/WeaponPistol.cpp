#include "stdafx.h"
#include "weaponpistol.h"
#include "WeaponHUD.h"
#include "ParticlesObject.h"
#include "actor.h"

CWeaponPistol::CWeaponPistol(LPCSTR name) : CWeaponCustomPistol(name)
{
	m_eSoundClose		= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING /*| eSoundType*/);
	m_opened = false;
	m_bPending = false;
}

CWeaponPistol::~CWeaponPistol(void)
{
}

void CWeaponPistol::net_Destroy()
{
	inherited::net_Destroy();

	// sounds
	HUD_SOUND::DestroySound(sndClose);
	HUD_SOUND::DestroySound(sndReloadEmpty);
}


void CWeaponPistol::Load	(LPCSTR section)
{
	inherited::Load		(section);

	HUD_SOUND::LoadSound(section, "snd_close", sndClose, m_eSoundClose);
	HUD_SOUND::LoadSound(section, "snd_reload_empty", sndReloadEmpty, m_eSoundClose,false);
	if (sndReloadEmpty.sounds.empty())
		HUD_SOUND::LoadSound(section, "snd_reload", sndReloadEmpty, m_eSoundClose);

	animGet				(mhud_pistol.mhud_empty,		pSettings->r_string(*hud_sect, "anim_empty"));
	animGet				(mhud_pistol.mhud_shot_l,		pSettings->r_string(*hud_sect, "anim_shot_last"));
	animGet				(mhud_pistol.mhud_close,		pSettings->r_string(*hud_sect, "anim_close"));
	animGet				(mhud_pistol.mhud_show_empty,	pSettings->r_string(*hud_sect, "anim_draw_empty"));
	animGet				(mhud_pistol.mhud_reload_empty,	pSettings->r_string(*hud_sect, "anim_reload_empty"));

	string128			str;
	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_empty"),"_r");
	animGet				(mhud_pistol_r.mhud_empty,		str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_shot_last"),"_r");
	animGet				(mhud_pistol_r.mhud_shot_l,		str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_close"),"_r");
	animGet				(mhud_pistol_r.mhud_close,		str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_draw_empty"),"_r");
	animGet				(mhud_pistol_r.mhud_show_empty,	str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_reload_empty"),"_r");
	animGet				(mhud_pistol_r.mhud_reload_empty,	str);



	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_idle"),"_r");
	animGet				(wm_mhud_r.mhud_idle,	str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_reload"),"_r");
	animGet				(wm_mhud_r.mhud_reload,	str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_draw"),"_r");
	animGet				(wm_mhud_r.mhud_show,	str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_holster"),"_r");
	animGet				(wm_mhud_r.mhud_hide,	str);

	strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_shoot"),"_r");
	animGet				(wm_mhud_r.mhud_shots,	str);

	if(IsZoomEnabled()){
		strconcat(sizeof(str),str,pSettings->r_string(*hud_sect, "anim_idle_aim"),"_r");
		animGet				(wm_mhud_r.mhud_idle_aim,		str);
	}

	LPCSTR animName = "anim_idle_moving_empty"; //пойдем на хитрости, дабы не добавлять анимацию для всего оружия
	if (pSettings->line_exist(*hud_sect, animName))
	{
		animGet(pre_anim_idle_moving_empty, pSettings->r_string(*hud_sect, animName), *hud_sect, animName);
	}
	animName = "anim_idle_sprint_empty";
	if (pSettings->line_exist(*hud_sect, animName))
	{
		animGet(pre_anim_idle_sprint_empty, pSettings->r_string(*hud_sect, animName), *hud_sect, animName);
	}
	pre_anim_idle_moving = mhud.anim_idle_moving; //закешируем обшие анимации
	pre_anim_idle_sprint = mhud.mhud_idle_sprint; //закешируем обшие анимации
}

void CWeaponPistol::OnH_B_Chield		()
{
	inherited::OnH_B_Chield		();
	m_opened = false;
}

void CWeaponPistol::PlayAnimShow	()
{
	VERIFY(GetState()==eShowing);
	if(iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;
		
	if(m_opened){ 
		CWeaponPistol::WWPMotions& m = wwpm_current();
		m_pHUD->animPlay(random_anim(m.mhud_show_empty),FALSE, this, GetState());
	}else{ 
		CWeaponMagazined::SWMmotions& m = swm_current();
		m_pHUD->animPlay(random_anim(m.mhud_show),FALSE, this, GetState());
	}
}

void CWeaponPistol::PlayAnimIdle	()
{
	VERIFY(GetState()==eIdle);
	MotionSVec *anim = nullptr;
	bool movingSprint = false;
	bool movingNormal = false;
	if (CActor* pActor = smart_cast<CActor*>(H_Parent()))
	{
		if (pActor->is_sprint())
			movingSprint = true;
		else if (pActor->AnyMove())
			movingNormal = true;
	}
	if (m_opened) //если пустой ствол
	{

		if (!movingSprint && !movingNormal)
			anim = &wwpm_current().mhud_empty; //типа стоим на месте
		else
		{
			if (movingNormal) //при обычном шаге
			{
				if (pre_anim_idle_moving_empty.empty()) //dirty hack если нет анимации для ходьбы с пустым стволом - сбросим анимации для inherited::PlayAnimIdle() 
					mhud.anim_idle_moving.clear(); //дабы включилась движковая раскачка
				else if (mhud.anim_idle_moving.empty()) //если есть - будем играть все, на всякий случай восстановим из кеша
					mhud.anim_idle_moving = pre_anim_idle_moving;
				anim = &pre_anim_idle_moving_empty; //попробуем проиграть анимацию движения с пустым стволом
			}
			else if (movingSprint) //при беге
			{
				if (pre_anim_idle_sprint_empty.empty()) //dirty hack если нет анимации для бега с пустым стволом - сбросим анимации для inherited::PlayAnimIdle() 
					mhud.mhud_idle_sprint.clear(); //дабы включилась движковая раскачка при беге
				else if (mhud.mhud_idle_sprint.empty()) //если есть - будем играть все, на всякий случай восстановим из кеша
					mhud.mhud_idle_sprint = pre_anim_idle_sprint_empty;
				anim = &pre_anim_idle_sprint_empty;
			}

			
		}
	}
	else
	{
		CActor* A = smart_cast<CActor*>(H_Parent());
		if (A && A->Holder())
			anim = (IsZoomed()) ? &wm_mhud_r.mhud_idle_aim : &wm_mhud_r.mhud_idle;
		if (!anim || anim->empty())
		{
			if (movingNormal)
			{
				if (mhud.anim_idle_moving.empty()) //восстановим из кеша, если надо обычную анимацию движения
					mhud.anim_idle_moving = pre_anim_idle_moving;
				anim = &mhud.anim_idle_moving;
			}
			else if (movingSprint) //при беге
			{
				if (mhud.mhud_idle_sprint.empty()) //восстановим из кеша, если надо обычную анимацию движения
					mhud.mhud_idle_sprint = pre_anim_idle_sprint;
				anim = &mhud.mhud_idle_sprint;
			}

		}
	}
	if (anim && !anim->empty())
		m_pHUD->animPlay(random_anim(*anim), TRUE, nullptr, GetState());
	else 
		inherited::PlayAnimIdle();
}

void CWeaponPistol::PlayAnimReload	()
{	
	VERIFY(GetState()==eReload);
	if(m_opened)
	{ 
		CWeaponPistol::WWPMotions& m = wwpm_current();
		m_pHUD->animPlay(random_anim(m.mhud_reload_empty), TRUE, this, GetState());
	}
	else
	{
		CWeaponMagazined::SWMmotions& m = swm_current();
		m_pHUD->animPlay(random_anim(m.mhud_reload), TRUE, this, GetState());
	}
	
	m_opened = false;		
}


void CWeaponPistol::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);
	if(m_opened) 
	{
		PlaySound			(sndClose,get_LastFP());
		CWeaponPistol::WWPMotions& m = wwpm_current();
		m_pHUD->animPlay	(random_anim(m.mhud_close), TRUE, this, GetState());
	} 
	else 
		inherited::PlayAnimHide();
}

void CWeaponPistol::PlayAnimShoot	()
{
	VERIFY(GetState()==eFire || GetState()==eFire2);
	if(iAmmoElapsed > 1) 
	{
		CWeaponMagazined::SWMmotions& m = swm_current();
		m_pHUD->animPlay	(random_anim(m.mhud_shots), FALSE, this, GetState());
		m_opened = false;
	}
	else 
	{
		CWeaponPistol::WWPMotions& m = wwpm_current();
		m_pHUD->animPlay	(random_anim(m.mhud_shot_l), FALSE, this, GetState()); 
		m_opened = true; 
	}
}

void CWeaponPistol::PlayReloadSound()
{
	if (m_opened)
		PlaySound(sndReloadEmpty, get_LastFP());
	else
		inherited::PlayReloadSound();
}


void CWeaponPistol::switch2_Reload()
{
//.	if(GetState()==eReload) return;
	inherited::switch2_Reload();
}

void CWeaponPistol::OnAnimationEnd(u32 state)
{
	if(state == eHiding && m_opened) 
	{
		m_opened = false;
//		switch2_Hiding();
	} 
	inherited::OnAnimationEnd(state);
}

void CWeaponPistol::OnShot		()
{
	// Sound
	PlaySound		(*m_pSndShotCurrent,get_LastFP());

	AddShotEffector	();
	
	PlayAnimShoot	();

	// Shell Drop
	Fvector vel; 
	PHGetLinearVell(vel);
	OnShellDrop					(get_LastSP(),  vel);

	// Огонь из ствола
	
	StartFlameParticles	();
	R_ASSERT2(!m_pFlameParticles || !m_pFlameParticles->IsLooped(),
			  "can't set looped particles system for shoting with pistol");
	
	//дым из ствола
	StartSmokeParticles	(get_LastFP(), vel);
}

void CWeaponPistol::UpdateSounds()
{
	inherited::UpdateSounds();

	if (sndClose.playing()) sndClose.set_position	(get_LastFP());
	if (sndReloadEmpty.playing()) sndReloadEmpty.set_position(get_LastFP());
}

CWeaponPistol::WWPMotions&	 CWeaponPistol::wwpm_current	()
{
	CActor* A = smart_cast<CActor*>(H_Parent());
	if(A && A->Holder()){	
//		Msg("right-hand animation playing");
		return				mhud_pistol_r;
	}
//	Msg("double-hands animation playing");
	return					mhud_pistol;
}

CWeaponMagazined::SWMmotions&	 CWeaponPistol::swm_current	()
{
	CActor* A = smart_cast<CActor*>(H_Parent());
	if(A && A->Holder()){
//.		Msg("right-hand animation playing");
		return				wm_mhud_r;
	}
//.	Msg("double-hands animation playing");
	return					mhud;
}
