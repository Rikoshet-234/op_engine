#include "stdafx.h"

#include "stdafx.h"
#include <dinput.h>
#include "../HUDManager.h"
#include "UICustomEdit.h"
#include "../../LightAnimLibrary.h"

static enum EInputCharset
{
	EIC_English = 0,
	EIC_Russian,
	EIC_Ukrainian,

	EIC_Count,
} gs_currentCharSet = EIC_English;

static struct SKeyMapping
{
	int key;
	char ch[EIC_Count*2];
} gs_keyMapping[] = 
{ 
	{ DIK_A,			{ 'a', 'A', 'ф', 'Ф', 'ф', 'Ф' }}, 
	{ DIK_B,			{ 'b', 'B', 'и', 'И', 'и', 'И' }}, 
	{ DIK_C,			{ 'c', 'C', 'с', 'С', 'с', 'С' }}, 
	{ DIK_D,			{ 'd', 'D', 'в', 'В', 'в', 'В' }}, 
	{ DIK_E,			{ 'e', 'E', 'у', 'У', 'у', 'У' }}, 
	{ DIK_F,			{ 'f', 'F', 'а', 'А', 'а', 'А' }}, 
	{ DIK_G,			{ 'g', 'G', 'п', 'П', 'п', 'П' }}, 
	{ DIK_H,			{ 'h', 'H', 'р', 'Р', 'р', 'Р' }}, 
	{ DIK_I,			{ 'i', 'I', 'ш', 'Ш', 'ш', 'Ш' }}, 
	{ DIK_J,			{ 'j', 'J', 'о', 'О', 'о', 'О' }}, 
	{ DIK_K,			{ 'k', 'K', 'л', 'Л', 'л', 'Л' }}, 
	{ DIK_L,			{ 'l', 'L', 'д', 'Д', 'д', 'Д' }}, 
	{ DIK_M,			{ 'm', 'M', 'ь', 'Ь', 'ь', 'Ь' }}, 
	{ DIK_N,			{ 'n', 'N', 'т', 'Т', 'т', 'Т' }}, 
	{ DIK_O,			{ 'o', 'O', 'щ', 'Щ', 'щ', 'Щ' }}, 
	{ DIK_P,			{ 'p', 'P', 'з', 'З', 'з', 'З' }}, 
	{ DIK_Q,			{ 'q', 'Q', 'й', 'Й', 'й', 'Й' }}, 
	{ DIK_R,			{ 'r', 'R', 'к', 'К', 'к', 'К' }}, 
	{ DIK_S,			{ 's', 'S', 'ы', 'Ы', 'і', 'І' }}, 
	{ DIK_T,			{ 't', 'T', 'е', 'Е', 'е', 'Е' }}, 
	{ DIK_U,			{ 'u', 'U', 'г', 'Г', 'г', 'Г' }}, 
	{ DIK_V,			{ 'v', 'V', 'м', 'М', 'м', 'М' }}, 
	{ DIK_W,			{ 'w', 'W', 'ц', 'Ц', 'ц', 'Ц' }}, 
	{ DIK_X,			{ 'x', 'X', 'ч', 'Ч', 'ч', 'Ч' }}, 
	{ DIK_Y,			{ 'y', 'Y', 'н', 'Н', 'н', 'Н' }}, 
	{ DIK_Z,			{ 'z', 'Z', 'я', 'Я', 'я', 'Я' }},
	{ DIK_0,			{ '0', ')', '0', ')', '0', ')' }}, 
	{ DIK_1,			{ '1', '!', '1', '!', '1', '!' }}, 
	{ DIK_2,			{ '2', '@', '2', '@', '2', '@' }}, 
	{ DIK_3,			{ '3', '#', '3', '#', '3', '#' }}, 
	{ DIK_4,			{ '4', '$', '4', '$', '4', '$' }}, 
	{ DIK_5,			{ '5', '%', '5', '%', '5', '%' }}, 
	{ DIK_6,			{ '6', '^', '6', '^', '6', '^' }}, 
	{ DIK_7,			{ '7', '&', '7', '&', '7', '&' }},
	{ DIK_8,			{ '8', '*', '8', '*', '8', '*' }}, 
	{ DIK_9,			{ '9', '(', '9', '(', '9', '(' }},
	{ DIK_LBRACKET,		{ '[', '{', 'х', 'Х', 'х', 'Х' }},
	{ DIK_RBRACKET,		{ ']', '}', 'ъ', 'Ъ', 'ї', 'Ї' }},
	{ DIK_APOSTROPHE,	{ '\'','"', 'э', 'Э', 'є', 'Є' }},
	{ DIK_COMMA,		{ ',', '<', 'б', 'Б', 'б', 'Б' }},
	{ DIK_PERIOD,		{ '.', '>', 'ю', 'Ю', 'ю', 'Ю' }},
	{ DIK_SLASH,		{ '/', '?', '.', ',', '.', ',' }},
	{ DIK_BACKSLASH,	{ '\\','|', '\\','/', '\\','/' }},
	{ DIK_SEMICOLON,	{ ';', ':', 'ж', 'Ж', 'ж', 'Ж' }},
};

static xr_map<u32, char> gs_DIK2CHR;

static inline u32 makeKey(int key, size_t chset, bool shift)
{
	return ((u32)(chset * 2 + (shift ? 1 : 0)) << 16) | (u32)key;
}

CUICustomEdit::CUICustomEdit()
{
	m_max_symb_count		= u32(-1);

	//! Initialize table only once
	if (gs_DIK2CHR.empty())
	{
		for (size_t i = 0; i < ARRAYSIZE(gs_keyMapping); ++i)
		{
			const SKeyMapping& km = gs_keyMapping[i];
			for(size_t j = 0; j < EIC_Count; ++j)
			{
				gs_DIK2CHR[makeKey(km.key, j, false)] = km.ch[j * 2];
				gs_DIK2CHR[makeKey(km.key, j, true)]  = km.ch[j * 2 + 1];
			}
		}
	}
	m_bShift = false;
	m_bControl = false;
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

void CUICustomEdit::Init(float x, float y, float width, float height){
	CUIWindow::Init(x,y,width,height);
	m_lines.SetWndSize(m_wndSize);
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
	if (m_bFocusByDbClick)
	{
		if(mouse_action == WINDOW_LBUTTON_DB_CLICK && !m_bInputFocus)
		{
			GetParent()->SetKeyboardCapture(this, true);
			m_bInputFocus = true;
			m_iKeyPressAndHold = 0;

			m_lines.MoveCursorToEnd();
		}
	}

	if(mouse_action == WINDOW_LBUTTON_DOWN && !m_bInputFocus)
	{
		GetParent()->SetKeyboardCapture(this, true);
		m_bInputFocus = true;
		m_iKeyPressAndHold = 0;

		m_lines.MoveCursorToEnd();
	}
	return false;
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
	xr_map<u32, char>::iterator it;
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
		break;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		m_bShift = true;
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
	case DIK_SPACE:
		out_me = ' ';
		break;
	case DIK_MINUS:
		out_me = m_bShift ? '_' : '-';	break;
	case DIK_EQUALS:
		out_me = m_bShift ? '+' : '=';	break;
	//! jarni: must be last, just before default, to catch case when control is not set
	case DIK_E:
		if (m_bControl)
		{
			gs_currentCharSet = EIC_English;
			break;
		}
	case DIK_R:
		if (m_bControl)
		{
			gs_currentCharSet = EIC_Russian;
			break;
		}
	case DIK_U:
		if (m_bControl)
		{
			gs_currentCharSet = EIC_Ukrainian;
			break;
		}
	default:
		it = gs_DIK2CHR.find(makeKey(dik, gs_currentCharSet, m_bShift));

		//нажата клавиша с буквой 
		if (gs_DIK2CHR.end() != it){
			AddLetter((*it).second);
			bChanged = true;
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

		u32 cur_time = Device.TimerAsync();

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