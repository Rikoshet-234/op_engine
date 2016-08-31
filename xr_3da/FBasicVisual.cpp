// IRender_Visual.cpp: implementation of the IRender_Visual class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#ifndef _EDITOR
	#include "render.h"
#endif    
#include "fbasicvisual.h"
#include "fmesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IRender_Mesh::~IRender_Mesh()		
{ 
	_RELEASE(p_rm_Vertices); 
	_RELEASE(p_rm_Indices);		
}

IRender_Visual::IRender_Visual		()
{
	Type				= 0;
	shader				= 0;
	vis.clear			();
}

IRender_Visual::~IRender_Visual		()
{
}

void IRender_Visual::Release		()
{
}

CStatTimer						tscreate;

void IRender_Visual::Load		(const char* N, IReader *data, u32 )
{
#ifdef DEBUG
	dbg_name	= N;
#endif

	// header
	VERIFY		(data);
	ogf_header	hdr;
	 /*if (N)
		 if (std::string(N).find("wpn_f_1")!=std::string::npos)
		 {
			 if (std::string(N).find("grenadier")!=std::string::npos)
				 int a=0;
		 }*/

	if (data->r_chunk_safe(OGF_HEADER,&hdr,sizeof(hdr)))
	{
		R_ASSERT2			(hdr.format_version==xrOGF_FormatVersion, "Invalid visual version");
		Type				= hdr.type;
		if (hdr.shader_id)	shader	= ::Render->getShader	(hdr.shader_id);
		vis.box.set			(hdr.bb.min,hdr.bb.max	);
		vis.sphere.set		(hdr.bs.c,	hdr.bs.r	);
	} else {
		FATAL				("Invalid visual");
	}

	// Shader
	bool error=false;
	//if (data->find_chunk(OGF_TEXTURE)) {
	if (data->find_chunk_safe(OGF_TEXTURE,error)) {
		if (error)
		{
			std::string meshName="sorry, visual path is unknown";
			if (N)
				meshName=N;
			Msg("! Invalid internal structure in visual [%s]",meshName.c_str());
			FATAL("ENGINE CRASH: See details in log");
		}
		string256		fnT,fnS;
		data->r_stringZ	(fnT,sizeof(fnT));
		data->r_stringZ	(fnS,sizeof(fnS));
		shader.create	(fnS,fnT);
	}
	

	// desc
#ifdef _EDITOR
	if (data->find_chunk(OGF_S_DESC)) 
		desc.Load		(*data);
#endif
}

#define PCOPY(a)	a = pFrom->a
void	IRender_Visual::Copy(IRender_Visual *pFrom)
{
	PCOPY(Type);
	PCOPY(shader);
	PCOPY(vis);
#ifdef _EDITOR
	PCOPY(desc);
#endif
#ifdef DEBUG
	PCOPY(dbg_name);
#endif
}
