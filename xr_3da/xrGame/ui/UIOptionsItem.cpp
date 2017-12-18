#include "StdAfx.h"
#include "UIOptionsItem.h"
#include "UIOptionsManager.h"
#include "../../xr_ioconsole.h"
#include "../GamePersistent.h"

CUIOptionsManager CUIOptionsItem::m_optionsManager;

CUIOptionsItem::~CUIOptionsItem()
{
	m_optionsManager.UnRegisterItem(this);
}

void CUIOptionsItem::Register(const char* entry, const char* group)
{
	m_optionsManager.RegisterItem	(this, group);
	m_entry							= entry;	
}

void CUIOptionsItem::SendMessage2Group(const char* group, const char* message)
{
	m_optionsManager.SendMessage2Group(group,message);
}

void CUIOptionsItem::OnMessage(const char* message)
{
	// do nothing
}

LPCSTR CUIOptionsItem::GetOptStringValue()
{
	return Console->GetString(m_entry.c_str());
}

void CUIOptionsItem::SaveOptStringValue(const char* val)
{
	xr_string command	= m_entry;
	command				+= " ";
	command				+= val;
	Console->Execute	(command.c_str());
}

void CUIOptionsItem::GetOptIntegerValue(int& val, int& min, int& max)
{
	Console->GetInteger(m_entry.c_str(), val, min, max);
}

void CUIOptionsItem::SaveOptIntegerValue(int val)
{
	string512			command;
	sprintf_s				(command,512, "%s %d", m_entry.c_str(), val);
	Console->Execute	(command);
}


void CUIOptionsItem::GetOptFloatValue(float& val, float& min, float& max)
{
	Console->GetFloat(m_entry.c_str(), val, min, max);
}

void CUIOptionsItem::SaveOptFloatValue(float val)
{
	string512			command;
	sprintf_s				(command,512, "%s %f", m_entry.c_str(), val);
	Console->Execute	(command);
}

bool CUIOptionsItem::GetOptBoolValue()
{
	BOOL val;
	Console->GetBool(m_entry.c_str(), val);
	return val ? true : false;
}

void CUIOptionsItem::SaveOptBoolValue(bool val)
{
	string512			command;
	sprintf_s				(command, "%s %s", m_entry.c_str(), (val)?"on":"off");
	Console->Execute	(command);
}

char* CUIOptionsItem::GetOptTokenValue()
{
	char* tokenValue= Console->GetToken(m_entry.c_str());
	if (m_entry == "font_profile")
		return const_cast<LPSTR>(pSettings->r_string("font_profiles", tokenValue));
	else if (m_entry == "g_lang")
		return const_cast<LPSTR>(pSettings->r_string("languages", tokenValue));
	return tokenValue;
}

xr_token* CUIOptionsItem::GetOptToken()
{
	return Console->GetXRToken(m_entry.c_str());
}

xr_vector<xr_token>* CUIOptionsItem::GetOptVectorToken()
{
	return Console->GetXRVectorTokens(m_entry.c_str());
}

void CUIOptionsItem::SaveOptTokenValue(const char* val){
	SaveOptStringValue(val);
}

extern bool IsMainMenuActive();

void CUIOptionsItem::SaveValue() {
	if (m_entry == "vid_mode" ||
		m_entry == "_preset" ||
		m_entry == "rs_fullscreen" ||
		m_entry == "rs_fullscreen" ||
		m_entry == "r__supersample" ||
		m_entry == "rs_refresh_60hz" ||
		m_entry == "rs_no_v_sync" ||
		m_entry == "texture_lod")
		m_optionsManager.DoVidRestart();

	if (m_entry == "font_profile")
		m_optionsManager.DoFontRestart();
	if (m_entry == "g_lang")
		m_optionsManager.DoLangRestart();

	if (/*m_entry == "snd_freq" ||*/ m_entry == "snd_efx")
		m_optionsManager.DoSndRestart();
}


