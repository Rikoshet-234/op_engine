#include "pch_script.h"
#include "phrasedialog.h"
#include "phrasedialogmanager.h"
#include "gameobject.h"
#include "ai_debug.h"
#include "ui/xrUIXmlParser.h"
#include "UIGameSP.h"
#include "Level.h"
#include "HUDManager.h"
#include "ui/UITalkWnd.h"

SPhraseDialogData::SPhraseDialogData ()
{
	m_PhraseGraph.clear	();
	m_iPriority			= 0;
	b_bForceReload=false;
}

SPhraseDialogData::~SPhraseDialogData ()
{}



CPhraseDialog::CPhraseDialog()
{
	m_bFirstIsSpeaking=true;
	m_bFinished = false;
	m_pSpeakerFirst = nullptr;
	m_pSpeakerSecond = nullptr;
	m_DialogId = nullptr;
}

CPhraseDialog::~CPhraseDialog()
{
}


void CPhraseDialog::Init(CPhraseDialogManager* speaker_first, CPhraseDialogManager* speaker_second)
{
	THROW(!IsInited());

	m_pSpeakerFirst		= speaker_first;
	m_pSpeakerSecond	= speaker_second;

	m_SaidPhraseID		= "";
	m_PhraseVector.clear();

	CPhraseGraph::CVertex* phrase_vertex = data()->m_PhraseGraph.vertex("0");
	THROW(phrase_vertex);
	m_PhraseVector.push_back(phrase_vertex->data());

	m_bFinished			= false;
	m_bFirstIsSpeaking	= true;
}

//обнуляем все связи
void CPhraseDialog::Reset ()	
{
}

CPhraseDialogManager* CPhraseDialog::OurPartner	(CPhraseDialogManager* dialog_manager) const
{
	if(FirstSpeaker() == dialog_manager)
		return SecondSpeaker();
	else
		return FirstSpeaker();
}


CPhraseDialogManager* CPhraseDialog::CurrentSpeaker()	const 
{
	return FirstIsSpeaking()?m_pSpeakerFirst:m_pSpeakerSecond;
}
CPhraseDialogManager* CPhraseDialog::OtherSpeaker	()	const 
{
	return (!FirstIsSpeaking())?m_pSpeakerFirst:m_pSpeakerSecond;
}


//предикат для сортировки вектора фраз
static bool PhraseGoodwillPred(const CPhrase* phrase1, const CPhrase* phrase2)
{
	return phrase1->GoodwillLevel()>phrase2->GoodwillLevel();
}

bool CPhraseDialog::SayPhrase (DIALOG_SHARED_PTR& phrase_dialog, const shared_str& phrase_id)
{
	THROW(phrase_dialog->IsInited());

	phrase_dialog->m_SaidPhraseID = phrase_id;

	bool first_is_speaking = phrase_dialog->FirstIsSpeaking();
	phrase_dialog->m_bFirstIsSpeaking = !phrase_dialog->m_bFirstIsSpeaking;

	const CGameObject*	pSpeakerGO1 = smart_cast<const CGameObject*>(phrase_dialog->FirstSpeaker());	VERIFY(pSpeakerGO1);
	const CGameObject*	pSpeakerGO2 = smart_cast<const CGameObject*>(phrase_dialog->SecondSpeaker());	VERIFY(pSpeakerGO2);
	if(!first_is_speaking) std::swap(pSpeakerGO1, pSpeakerGO2);

	CPhraseGraph::CVertex* phrase_vertex = phrase_dialog->data()->m_PhraseGraph.vertex(phrase_dialog->m_SaidPhraseID);
	THROW(phrase_vertex);

	CPhrase* last_phrase = phrase_vertex->data();

	//вызвать скриптовую присоединенную функцию 
	//активируется после сказанной фразы
	//первый параметр - тот кто говорит фразу, второй - тот кто слушает
	last_phrase->m_PhraseScript.Action(pSpeakerGO1, pSpeakerGO2, *phrase_dialog->m_DialogId, phrase_id.c_str() );

	//больше нет фраз, чтоб говорить
	phrase_dialog->m_PhraseVector.clear();
	if(phrase_vertex->edges().empty())
	{
		phrase_dialog->m_bFinished = true;
	}
	else
	{
		//обновить список фраз, которые сейчас сможет говорить собеседник
		for(xr_vector<CPhraseGraph::CEdge>::const_iterator it = phrase_vertex->edges().begin();
			it != phrase_vertex->edges().end();
		    ++it)
		{
			const CPhraseGraph::CEdge& edge = *it;
			CPhraseGraph::CVertex* next_phrase_vertex = phrase_dialog->data()->m_PhraseGraph.vertex(edge.vertex_id());
			THROW						(next_phrase_vertex);
			shared_str next_phrase_id	= next_phrase_vertex->vertex_id();
			if(next_phrase_vertex->data()->m_PhraseScript.Precondition(pSpeakerGO2, pSpeakerGO1, *phrase_dialog->m_DialogId, phrase_id.c_str(), next_phrase_id.c_str()))
			{
				phrase_dialog->m_PhraseVector.push_back(next_phrase_vertex->data());
#ifdef DEBUG
				if(psAI_Flags.test(aiDialogs)){
					LPCSTR phrase_text = next_phrase_vertex->data()->GetText();
					shared_str id = next_phrase_vertex->data()->GetID();
					Msg("----added phrase text [%s]phrase_id=[%s] id=[%s] to dialog [%s]",phrase_text, phrase_id, id, *phrase_dialog->m_DialogId);
				}
#endif
			}

		}

		if (phrase_dialog->m_PhraseVector.empty())
		{
			string1024 text;
			sprintf_s(text,"No available phrase to say in [%s] , after [%s], context:\n %s",
				phrase_dialog->m_DialogId.c_str(),phrase_id.c_str(),DialogDebugContext());

			Msg("! ERROR CPhraseDialog::SayPhrase: %s", text);
			CPhrase  *cap = phrase_dialog->AddPhrase_script (text, "error_phrase", *phrase_id, 0);
			phrase_dialog->m_PhraseVector.push_back(cap);
		}
		/*R_ASSERT2	(
			!phrase_dialog->m_PhraseVector.empty(),
			make_string(
				"No available phrase to say, dialog[%s]",
				*phrase_dialog->m_DialogId
			)
		);*/

		//упорядочить списко по убыванию благосклонности
		std::sort(phrase_dialog->m_PhraseVector.begin(),
				 phrase_dialog->m_PhraseVector.end(), PhraseGoodwillPred);
	}



	//сообщить CDialogManager, что сказана фраза
	//и ожидается ответ
	if(first_is_speaking)
		phrase_dialog->SecondSpeaker()->ReceivePhrase(phrase_dialog);
	else
		phrase_dialog->FirstSpeaker()->ReceivePhrase(phrase_dialog);


	return phrase_dialog?!phrase_dialog->m_bFinished:true;
}

LPCSTR CPhraseDialog::GetPhraseText	(const shared_str& phrase_id, bool current_speaking)
{
	
	CPhraseGraph::CVertex* phrase_vertex = data()->m_PhraseGraph.vertex(phrase_id);
	if (!phrase_vertex)
	{
		Msg("! ERROR cannot get phrase graph for phrase_id[%s]", phrase_id.c_str());
		FATAL("ENGINE Crush. See log for details.");
	}

	return phrase_vertex->data()->GetText();
}

LPCSTR CPhraseDialog::DialogCaption()
{
	return data()->m_sCaption.size()?*data()->m_sCaption:GetPhraseText("0");
}


int	 CPhraseDialog::Priority()
{
	return data()->m_iPriority;
}


void CPhraseDialog::Load(shared_str dialog_id)
{
	m_DialogId = dialog_id;
	bool need_load=inherited_shared::start_load_shared(m_DialogId); //начинаем загрузку
	if (need_load) //свеже созданное
		inherited_shared::load_shared(m_DialogId, nullptr);
	else if (GetDialogForceReload()) //уже создавалось раньше
	{
		CUIGameSP* ui_sp = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
		if (ui_sp && ui_sp->TalkMenu->GetInitState()) //читать только если идет инициализация окна диалога. в других Load - загружать наново не надо
		{
			ITEM_DATA item_data = *id_to_index::GetById(m_DialogId); //перечитаем xml часть диалога
			std::string file_name=item_data._xml->m_xml_file_name;
			const size_t sidx = file_name.rfind('\\');
			if (std::string::npos != sidx)
				file_name=file_name.substr(sidx+1,file_name.length());
			item_data._xml->ReInit(CONFIG_PATH, GAME_PATH, file_name.c_str()); //физически распарсим наново xml документ
			data()->SetLoad(false);
			inherited_shared::load_shared(m_DialogId, nullptr); //перестроим грай диалогов на основании новой структуры xml
		}
	}
}

#include "script_engine.h"
#include "ai_space.h"

void CPhraseDialog::load_shared	(LPCSTR)
{
	const ITEM_DATA& item_data = *id_to_index::GetById(m_DialogId);

	CUIXml*		pXML		= item_data._xml;
	pXML->SetLocalRoot		(pXML->GetRoot());
	//loading from XML
	XML_NODE* dialog_node = pXML->NavigateToNode(id_to_index::GetTagName(), item_data.pos_in_file);
	THROW3(dialog_node, "dialog id=", *item_data.id);

	pXML->SetLocalRoot(dialog_node);


	SetPriority	( pXML->ReadAttribInt(dialog_node, "priority", 0) );

	//заголовок 
	SetCaption	( pXML->Read(dialog_node, "caption", 0, nullptr) );

	SetDialogForceReload(pXML->ReadAttribInt(dialog_node, "force_reload", 0)==1? true:false);
	//предикаты начала диалога
	data()->m_PhraseScript.Load(pXML, dialog_node);

	//заполнить граф диалога фразами
	data()->m_PhraseGraph.clear();

	XML_NODE* phrase_list_node = pXML->NavigateToNode(dialog_node, "phrase_list", 0);
	if(NULL == phrase_list_node){
		LPCSTR func = pXML->Read(dialog_node, "init_func", 0, "");

		luabind::functor<void>	lua_function;
		bool functor_exists = ai().script_engine().functor(func ,lua_function);
		THROW3(functor_exists, "Cannot find precondition", func);
		lua_function	(this);
		return;
	}

	int phrase_num = pXML->GetNodesNum(phrase_list_node, "phrase");
	THROW3(phrase_num, "dialog %s has no phrases at all", *item_data.id);

	pXML->SetLocalRoot(phrase_list_node);

#ifdef DEBUG // debug & mixed
	LPCSTR wrong_phrase_id = pXML->CheckUniqueAttrib(phrase_list_node, "phrase", "id");
	THROW3(wrong_phrase_id == NULL, *item_data.id, wrong_phrase_id);
#endif	

	//ищем стартовую фразу
	XML_NODE* phrase_node	= pXML->NavigateToNodeWithAttribute("phrase", "id", "0");
	THROW					(phrase_node);
	AddPhrase				(pXML, phrase_node, "0", "");
}

void CPhraseDialog::SetCaption	(LPCSTR str)
{
	data()->m_sCaption = str;
}

void CPhraseDialog::SetPriority	(int val)
{
	data()->m_iPriority = val;
}

void CPhraseDialog::SetDialogForceReload(bool value)
{
	data()->b_bForceReload=value;
}

bool CPhraseDialog::GetDialogForceReload()
{
	return data()->b_bForceReload;
}

CPhrase* CPhraseDialog::AddPhrase	(LPCSTR text, const shared_str& phrase_id, const shared_str& prev_phrase_id, int goodwil_level)
{
	CPhrase* phrase					= nullptr;
	CPhraseGraph::CVertex* _vertex	= data()->m_PhraseGraph.vertex(phrase_id);
	if(!_vertex) 
	{
		phrase						= xr_new<CPhrase>(); VERIFY(phrase);
		phrase->SetID				(phrase_id);

		phrase->SetText				(text);
		phrase->m_iGoodwillLevel	= goodwil_level;

		data()->m_PhraseGraph.add_vertex	(phrase, phrase_id);
	}

	if(prev_phrase_id != "")
	{
		auto edge=data()->m_PhraseGraph.vertex(prev_phrase_id);
		if (!edge)
			Msg("! ERROR can't add phrase to graph! Not exist previouse edge point! Dialog[%s] phrase_id[%s] prev_phrase_id[%s]",m_DialogId.c_str(),phrase_id.c_str(),prev_phrase_id.c_str());
		else
			data()->m_PhraseGraph.add_edge		(prev_phrase_id, phrase_id, 0.f);
	}
	
	return phrase;
}

static int sNextId = 0;
int getNextId() { return ++sNextId; }

CPhrase* CPhraseDialog::AddPhrase_script(LPCSTR text, LPCSTR phrase_id, LPCSTR prev_phrase_id, int goodwil_level)
{
	CPhrase* ph = AddPhrase(text, phrase_id, prev_phrase_id, goodwil_level);
	if (!ph)
	{
		string1024 error_text;
		sprintf_s(error_text,"Maybe duplicate phrase_id! dialog [%s] , phrase_id[%s] Not talkin again!!!! ",
				m_DialogId.c_str(),phrase_id);
		Msg("! ERROR CPhraseDialog::AddPhrase_script: %s.", error_text);
		string64 strId;
		sprintf_s(strId,"error_phrase_%i",getNextId());
		CPhrase  *error_phrase = AddPhrase_script (error_text, strId, prev_phrase_id, 0);
		return error_phrase;
	}
	return ph;
}

void CPhraseDialog::AddPhrase	(CUIXml* pXml, XML_NODE* phrase_node, const shared_str& phrase_id, const shared_str& prev_phrase_id)
{

	LPCSTR sText		= pXml->Read		(phrase_node, "text", 0, "");
	int		gw			= pXml->ReadInt		(phrase_node, "goodwill", 0, -10000);
	CPhrase* ph			= AddPhrase			(sText, phrase_id, prev_phrase_id, gw);
	if(!ph)				return;

	ph->m_PhraseScript.Load					(pXml, phrase_node);

	//фразы которые собеседник может говорить после этой
	int next_num = pXml->GetNodesNum(phrase_node, "next");
	for(int i=0; i<next_num; ++i)
	{
		LPCSTR next_phrase_id_str		= pXml->Read(phrase_node, "next", i, "");
		XML_NODE* next_phrase_node		= pXml->NavigateToNodeWithAttribute("phrase", "id", next_phrase_id_str);
		R_ASSERT2						(next_phrase_node, next_phrase_id_str );
//.		int next_phrase_id				= atoi(next_phrase_id_str);

		AddPhrase						(pXml, next_phrase_node, next_phrase_id_str, phrase_id);
	}
}

bool  CPhraseDialog::Precondition(const CGameObject* pSpeaker1, const CGameObject* pSpeaker2)
{
	return data()->m_PhraseScript.Precondition(pSpeaker1, pSpeaker2, m_DialogId.c_str(), "", "");
}

void   CPhraseDialog::InitXmlIdToIndex(LPCSTR& file_str, LPCSTR& tag_name)
{
	if(!tag_name)
		tag_name = "dialog";
	if(!file_str)
		file_str = pSettings->r_string("dialogs", "files");
}

bool CPhraseDialog::allIsDummy	()
{
	PHRASE_VECTOR_IT it = m_PhraseVector.begin();
	bool bAllIsDummy = true;
	for(;it!=m_PhraseVector.end();++it)
	{
		if( !(*it)->IsDummy() )
			bAllIsDummy=false;
	}

	return bAllIsDummy;
}
