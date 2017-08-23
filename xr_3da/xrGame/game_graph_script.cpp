////////////////////////////////////////////////////////////////////////////
//	Module 		: game_graph_script.cpp
//	Created 	: 02.11.2005
//  Modified 	: 02.11.2005
//	Author		: Dmitriy Iassenev
//	Description : Game graph class script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "game_graph.h"
#include "ai_space.h"

using namespace luabind;

const CGameGraph *get_game_graph		()
{
	return				(&ai().game_graph());
}

const CGameGraph::CHeader *get_header	(const CGameGraph *pself)
{
	return				(&pself->header());
}

bool get_accessible1					(const CGameGraph *pself, const u32 &vertex_id)
{
	return				(pself->accessible(vertex_id));
}

void get_accessible2					(const CGameGraph *pself, const u32 &vertex_id, bool value)
{
	pself->accessible	(vertex_id,value);
}

Fvector CVertex__level_point			(const CGameGraph::CVertex *vertex)
{
	THROW				(vertex);
	return				(vertex->level_point());
}

Fvector CVertex__game_point				(const CGameGraph::CVertex *vertex)
{
	THROW				(vertex);
	return				(vertex->game_point());
}

LPCSTR get_level_name_by_graph_id(GameGraph::_GRAPH_ID graph_id)
{
	auto level = ai().game_graph().header().levels().find(ai().game_graph().vertex(graph_id)->level_id());
	if (level!= ai().game_graph().header().levels().end())
	{
		return level->second.name().c_str();
	}
	return "";
}
#pragma optimize("s",on)
void CGameGraph::script_register		(lua_State *L)
{
	module(L)
	[
		def("game_graph",	&get_game_graph),

		class_<CGameGraph>("CGameGraph")
			.def("accessible",		&get_accessible1)
			.def("accessible",		&get_accessible2)
			.def("valid_vertex_id",	&CGameGraph::valid_vertex_id)
			.def("vertex",			&CGameGraph::vertex)
			.def("vertex_id",		&CGameGraph::vertex_id)
			.def("level_name_by_gvid",&get_level_name_by_graph_id),

		class_<CVertex>("GameGraph__CVertex")
			.def("level_point",		&CVertex__level_point)
			.def("game_point",		&CVertex__game_point)
			.def("level_id",		&CVertex::level_id)
			.def("level_vertex_id",	&CVertex::level_vertex_id)
	];
}
