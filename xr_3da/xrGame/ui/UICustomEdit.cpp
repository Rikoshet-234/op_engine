#include "stdafx.h"

#include "stdafx.h"
#include <dinput.h>
#include "../HUDManager.h"
#include "UICustomEdit.h"
#include "../RegistryFuncs.h"
#include "../../LightAnimLibrary.h"
#include "UIInventoryUtilities.h"
#include "../xrCore/xr_ini.h"

typedef xr_vector<xr_string> TLanguageNames;
static enum ELayoutSwitchShortcut
{
	ELSS_AltShift = 0,
	ELSS_CtrlShift
} gs_currentLayoutSwitchShortCut = ELSS_AltShift;

#define ENRUUA_ONLY
//#define USE_GLOBAL_LANGUAGE

static u8 gs_KBState[256];
static xr_vector<HKL> gs_hklList;
static TLanguageNames gs_langNames;

#ifdef USE_GLOBAL_LANGUAGE
static size_t gs_currentSelectedLanguage = 0;
#define CURRENT_SELECTED_LANGUAGE gs_currentSelectedLanguage
#else
#define CURRENT_SELECTED_LANGUAGE m_currentSelectedLanguage
#endif

CUICustomEdit::CUICustomEdit()
{
	m_max_symb_count		= u32(-1);

	//! Initialize table only once
	if (gs_hklList.empty())
	{
		//! Zero KB state list, particular keys will be set in appropriate key events
		ZeroMemory(gs_KBState, sizeof(gs_KBState));

		//! Find out which short cut switches languages
		DWORD layoutSwitchShortcut = 0;
		ReadRegistry_DWValue(false, "Keyboard Layout\\Toggle", "Hotkey", layoutSwitchShortcut);
		if (layoutSwitchShortcut == 2)
		{
			gs_currentLayoutSwitchShortCut = ELSS_CtrlShift;
		}
		else
		{
			gs_currentLayoutSwitchShortCut = ELSS_AltShift;
		}

		//! Get keyboard layouts from system
		xr_vector<HKL> hklList;
		hklList.resize(GetKeyboardLayoutList(0, NULL));
		GetKeyboardLayoutList(static_cast<int>(hklList.size()), &hklList[0]);
		
		//! Create list of input languages
		CURRENT_SELECTED_LANGUAGE = 0;
		gs_langNames.resize(hklList.size());
		gs_hklList.reserve(hklList.size());
		for(size_t i = 0, j = 0; i < hklList.size(); ++i)
		{
			ActivateKeyboardLayout(hklList[i], KLF_SETFORPROCESS);
			char layoutName[64] = { 0 };
			if (GetKeyboardLayoutName(layoutName))
			{
				xr_string key = "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\";
				key.append(layoutName);
				char layoutText[64];
				ReadRegistry_StrValue(true, key.c_str(), "Layout Text", layoutText);
				char c1 = static_cast<char>(toupper(layoutText[0]));
				char c2 = static_cast<char>(toupper(layoutText[1]));

				if (c1 == 'U' && c2 == 'S')
				{
					c1 = 'E';
					c2 = 'N';
					CURRENT_SELECTED_LANGUAGE = gs_hklList.size();
				}

#ifdef ENRUUA_ONLY
				if (c1 == 'E' && c2 == 'N' ||
					c1 == 'R' && c2 == 'U' ||
					c1 == 'U' && c2 == 'K')
#endif
				{
					gs_hklList.push_back(hklList[i]);
					gs_langNames[j].resize(3);
					gs_langNames[j][0] = c1;
					gs_langNames[j][1] = c2;
					gs_langNames[j][2] = 0;
					++j;
				}
			}
			else
			{
				//! Skip layout if we can't get it's name
			}
		}
		
		//! Shrink names list to actual size
		gs_langNames.resize(gs_hklList.size());
	}

#ifndef USE_GLOBAL_LANGUAGE
	m_currentSelectedLanguage = 0;
	for(size_t i = 0; i < gs_langNames.size(); ++i)
	{
		if (gs_langNames[i][0] == 'E' && gs_langNames[i][1] == 'N')
		{
			m_currentSelectedLanguage = i;
			break;
		}
	}
#endif
	m_bShift = false;
	m_bControl = false;
	m_bAlt = false;
	m_bCapital = false;
	m_bInputFocus = false;

	m_iKeyPressAndHold = 0;
	m_bHoldWaitMode = false;
   
	m_lines.SetVTextAlignment(valCenter);
	m_lines.SetColoringMode(false);
	m_lines.SetCutWordsMode(true);
	m_lines.SetUseNewLineMode(false);
	SetText("");
	m_textPos.set(3,0);
	m_bNumbersOnly = false;
	m_bFloatNumbers = false;
	m_bFocusByDbClick = false;

	m_textColor[0]=color_argb(255,235,219,185);
	m_textColor[1]=color_argb(255,100,100,100);

	AttachChild(&m_languageIcon);
}

CUICustomEdit::~CUICustomEdit()
{
}

void CUICustomEdit::SetTextColor(u32 color){
	m_textColor[0] = color;
}

void CUICustomEdit::SetTextColorD(u32 color){
	m_textColor[1] = color;
}

void CUICustomEdit::LoadSettings(LPCSTR path)
{
	m_path = path;
	std::replace(m_path.begin(), m_path.end(), ':', '_');
	
	IReader* fr = (FS.exist("$game_settings$","ceb.settings") == NULL) ? NULL : FS.r_open("$game_settings$","ceb.settings");
	if (fr)
	{
		CInifile cebSettings(fr);
#ifndef USE_GLOBAL_SETTINGS
		LPCSTR inputLanguage = cebSettings.line_exist(m_path.c_str(), "input_language") ? cebSettings.r_string(m_path.c_str(), "input_language") : NULL;
#else
		LPCSTR inputLanguage = cebSettings.line_exist("Global", "input_language") ? cebSettings.r_string("Global", "input_language") : NULL;
#endif
		if (inputLanguage)
		{
			const size_t inLength = xr_strlen(inputLanguage);
			for (TLanguageNames::size_type i = 0; i < gs_langNames.size(); ++i)
			{
				const size_t maxLen = std::max(gs_langNames[i].length(), inLength);
				if (0 == strncmp(gs_langNames[i].c_str(), inputLanguage, maxLen))//! Compare up to longest string to not match same substrings
				{
					CURRENT_SELECTED_LANGUAGE = i;
					m_languageIcon.SetText(!m_bNumbersOnly ? gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str() : "");
					break;
				}
			}
		}
		else
		{
			//! Input language not yet stored in settings file
		}
		fr->close();
	}
}

void CUICustomEdit::Init(float x, float y, float width, float height)
{
	CUIWindow::Init(x,y,width,height);
	m_lines.SetWndSize(m_wndSize);

	m_languageIcon.SetAutoDelete(false);
	m_languageIcon.SetColor(m_textColor[0]);
	m_languageIcon.TextureAvailable(false);
	m_languageIcon.SetText(gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str());
	m_languageIcon.SetWndPos(width - 7.f,2.0f);
	Fvector2 wndSize;
	wndSize.set(7.f, height-4.f);
	m_languageIcon.SetWndSize(wndSize);
}

void CUICustomEdit::SetLightAnim(LPCSTR lanim)
{
	if(lanim&&xr_strlen(lanim))
		m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim	= NULL;
}

void CUICustomEdit::SetPasswordMode(bool mode){
	m_lines.SetPasswordMode(mode);
}

void CUICustomEdit::OnFocusLost(){
	CUIWindow::OnFocusLost();
/*	//only for CDKey control
	if(m_bInputFocus)
	{
		m_bInputFocus = false;
		m_iKeyPressAndHold = 0;
		GetMessageTarget()->SendMessage(this,EDIT_TEXT_COMMIT,NULL);
	}
*/
}

void CUICustomEdit::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
//	if(pWnd == GetParent())
//	{
		//кто-то другой захватил клавиатуру
		if(msg == WINDOW_KEYBOARD_CAPTURE_LOST)
		{
			m_bInputFocus = false;
			m_iKeyPressAndHold = 0;
		}
//	}
}

bool CUICustomEdit::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_DB_CLICK || mouse_action == WINDOW_LBUTTON_DOWN || mouse_action == WINDOW_RBUTTON_DOWN)
	{
		Frect liRect = m_languageIcon.GetWndRect();
		if (liRect.in(x,y))
		{
			ChangeInputLanguage();
		}
	}

	if (m_bFocusByDbClick)
	{
		if(mouse_action == WINDOW_LBUTTON_DB_CLICK && !m_bInputFocus)
		{
			GetParent()->SetKeyboardCapture(this, true);
			m_bInputFocus = true;
			m_iKeyPressAndHold = 0;
			m_languageIcon.SetText(!m_bNumbersOnly ? gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str() : "");
			m_lines.MoveCursorToEnd();
		}
	}

	if(mouse_action == WINDOW_LBUTTON_DOWN && !m_bInputFocus)
	{
		GetParent()->SetKeyboardCapture(this, true);
		m_bInputFocus = true;
		m_iKeyPressAndHold = 0;
		m_languageIcon.SetText(!m_bNumbersOnly ? gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str() : "");
		m_lines.MoveCursorToEnd();
	}
	return true;
}


bool CUICustomEdit::OnKeyboard(int dik, EUIMessages keyboard_action)
{	
	if(!m_bInputFocus) 
		return false;
	if(keyboard_action == WINDOW_KEY_PRESSED)	
	{
		m_iKeyPressAndHold = dik;
		m_bHoldWaitMode = true;

		if(KeyPressed(dik))	return true;
	}
	else if(keyboard_action == WINDOW_KEY_RELEASED)	
	{
		if(m_iKeyPressAndHold == dik)
		{
			m_iKeyPressAndHold = 0;
			m_bHoldWaitMode = false;
		}
		if(KeyReleased(dik)) return true;
	}
	return false;
}

bool CUICustomEdit::KeyPressed(int dik)
{
	char out_me = 0;
	bool bChanged = false;
	switch(dik)
	{
	case DIK_LEFT:
	case DIKEYBOARD_LEFT:
		m_lines.DecCursorPos();		
		break;
	case DIK_RIGHT:
	case DIKEYBOARD_RIGHT:
		m_lines.IncCursorPos();		
		break;
	case DIK_LCONTROL:
	case DIK_RCONTROL:
		m_bControl = true;
		CheckSwitchInputLanguage();
		break;
	case DIK_LMENU:
	case DIK_RMENU:
		m_bAlt = true;
		CheckSwitchInputLanguage();
		break;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		m_bShift = true;
		CheckSwitchInputLanguage();
		break;
	case DIK_CAPITAL:
		m_bCapital = !m_bCapital;
		break;
	case DIK_ESCAPE:
		if (xr_strlen(GetText()) != 0)
		{
			SetText("");
			bChanged = true;
		}
		else
		{
			GetParent()->SetKeyboardCapture(this, false);
			m_bInputFocus = false;
			m_iKeyPressAndHold = 0;
		};
		break;
	case DIK_RETURN:
	case DIK_NUMPADENTER:
		GetParent()->SetKeyboardCapture(this, false);
		m_bInputFocus = false;
		m_iKeyPressAndHold = 0;
		GetMessageTarget()->SendMessage(this,EDIT_TEXT_COMMIT,NULL);
		break;
	case DIK_BACKSPACE:
		m_lines.DelLeftChar();
		bChanged = true;
		break;
	case DIK_DELETE:
	case DIKEYBOARD_DELETE:
		m_lines.DelChar();
		bChanged = true;
		break;
	default:
		{
			gs_KBState[VK_SHIFT] = m_bShift ? 0x80 : 0x00;
			gs_KBState[VK_CAPITAL] = m_bCapital ? 0x01 : 0x00;
			gs_KBState[VK_CONTROL] = m_bControl ? 0x80 : 0x00;
			gs_KBState[VK_MENU] = m_bAlt ? 0x80 : 0x00;

			UINT vk = MapVirtualKeyEx(dik, MAPVK_VSC_TO_VK_EX, gs_hklList[CURRENT_SELECTED_LANGUAGE]);
			WORD result = 0;
			if (ToAsciiEx(vk, dik, gs_KBState, &result, 0, gs_hklList[CURRENT_SELECTED_LANGUAGE]) == 1)
			{
				AddLetter(LOBYTE(result));
				bChanged = true;
			}
		}
		break;
	}

	if (m_bNumbersOnly)
	{
		if (strstr(m_lines.GetText(), "."))
			return true;
		if (('.' == out_me) && m_bFloatNumbers){
			AddChar(out_me);
			bChanged = true;
		}
	}
	else
		if(out_me){
			AddChar(out_me);
			bChanged = true;
		}

		if(bChanged)
			GetMessageTarget()->SendMessage(this,EDIT_TEXT_CHANGED,NULL);

		return true;
}

bool CUICustomEdit::KeyReleased(int dik)
{
	switch(dik)
	{
	case DIK_LCONTROL:
	case DIK_RCONTROL:
		m_bControl = false;
		return true;
	case DIK_LMENU:
	case DIK_RMENU:
		m_bAlt = false;
		return true;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		m_bShift = false;
		return true;
	}

	return true;
}

void CUICustomEdit::AddChar(char c)
{
	if(xr_strlen(m_lines.GetText()) >= m_max_symb_count)					return;

	float text_length	= m_lines.GetFont()->SizeOf_(m_lines.GetText());
	UI()->ClientToScreenScaledWidth		(text_length);

	if (!m_lines.GetTextComplexMode() && (text_length > GetWidth() - 1))	return;

	m_lines.AddCharAtCursor(c);
	m_lines.ParseText();
	if (m_lines.GetTextComplexMode())
	{
		if (m_lines.GetVisibleHeight() > GetHeight())
			m_lines.DelLeftChar();
	}
}

void CUICustomEdit::AddLetter(char c)
{
	if (m_bNumbersOnly)
	{
		if ((c >= '0' && c<='9'))
			AddChar(c);

		return;
	}
	AddChar(c);
}

//время для обеспечивания печатания
//символа при удерживаемой кнопке
#define HOLD_WAIT_TIME 400
#define HOLD_REPEAT_TIME 100

void CUICustomEdit::Update()
{
	if(m_bInputFocus)
	{	
		static u32 last_time; 

		u32 cur_time = Device.TimerAsync_MMT();

		if(m_iKeyPressAndHold)
		{
			if(m_bHoldWaitMode)
			{
				if(cur_time - last_time>HOLD_WAIT_TIME)
				{
					m_bHoldWaitMode = false;
					last_time = cur_time;
				}
			}
			else
			{
				if(cur_time - last_time>HOLD_REPEAT_TIME)
				{
					last_time = cur_time;
					KeyPressed(m_iKeyPressAndHold);
				}
			}
		}
		else
			last_time = cur_time;
	}

	m_lines.SetTextColor(m_textColor[IsEnabled()?0:1]);

	CUIWindow::Update();
}

void  CUICustomEdit::Draw()
{
	CUIWindow::Draw			();
	Fvector2				pos;
	GetAbsolutePos			(pos);
	m_lines.Draw			(pos.x + m_textPos.x, pos.y + m_textPos.y);
	
	if(m_bInputFocus)
	{ //draw cursor here
		Fvector2							outXY;
		
		outXY.x								= 0.0f;
		float _h				= m_lines.m_pFont->CurrentHeight_();
		UI()->ClientToScreenScaledHeight(_h);
		outXY.y								= pos.y + (GetWndSize().y - _h)/2.0f;

		float								_w_tmp;
		int i								= m_lines.m_iCursorPos;
		string256							buff;
		strncpy								(buff,m_lines.m_text.c_str(),i);
		buff[i]								= 0;
		_w_tmp								= m_lines.m_pFont->SizeOf_(buff);
		UI()->ClientToScreenScaledWidth		(_w_tmp);
		outXY.x								= pos.x + _w_tmp;
		
		_w_tmp								= m_lines.m_pFont->SizeOf_("-");
		UI()->ClientToScreenScaledWidth		(_w_tmp);
		UI()->ClientToScreenScaled			(outXY);

		m_lines.m_pFont->Out				(outXY.x, outXY.y, "_");
	}
	m_languageIcon.Draw();
}

void CUICustomEdit::SetText(LPCSTR str)
{
	CUILinesOwner::SetText(str);
}

const char* CUICustomEdit::GetText(){
	return CUILinesOwner::GetText();
}

void CUICustomEdit::Enable(bool status){
	CUIWindow::Enable(status);
	if (!status)
		SendMessage(this,WINDOW_KEYBOARD_CAPTURE_LOST);
}

void CUICustomEdit::SetNumbersOnly(bool status){
	m_bNumbersOnly = status;
}

void CUICustomEdit::SetFloatNumbers(bool status){
	m_bFloatNumbers = status;
}

void CUICustomEdit::CheckSwitchInputLanguage()
{
	if (m_bShift && (gs_currentLayoutSwitchShortCut == ELSS_AltShift ? m_bAlt : m_bControl))
	{
		ChangeInputLanguage();
	}
}

void CUICustomEdit::ChangeInputLanguage()
{
	CURRENT_SELECTED_LANGUAGE = (CURRENT_SELECTED_LANGUAGE + 1) % gs_hklList.size();
	m_languageIcon.SetText(!m_bNumbersOnly ? gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str() : "");

	if (!m_bNumbersOnly)
	{
		string_path fname; 
		FS.update_path(fname,"$game_settings$","ceb.settings");
		CInifile cebSettings(fname, FALSE, TRUE, TRUE);
#ifndef USE_GLOBAL_SETTINGS
		LPCSTR section_name = m_path.c_str();
#else
		LPCSTR section_name = "Global";
#endif
		cebSettings.w_string(section_name, "input_language", gs_langNames[CURRENT_SELECTED_LANGUAGE].c_str());
	}
	else
	{
		//! Do not store language settings for number only edit box
	}
	
}