#include "stdafx.h"
#include "pch_script.h"
#include "script_engine_export.h"
#include "../xr_3da/CMultiHUDs.h"
#include "multihuds_registrator.h"

CMultiHUDs* get_multihuds()
{
	return multiHUDs;
}

xr_string get_folder(CHUDProfile* profile)
{
	if (profile && profile->folder_path.size() > 0)
		return profile->folder_path.c_str();
	return nullptr;
}

xr_string get_short_description(CHUDProfile* profile)
{
	if (profile && profile->description_fn.size()>0)
	{
		IReader* F = FS.r_open(profile->description_fn.c_str());
		if (F)
		{
			xr_string text;
			F->r_string(text);
			FS.r_close(F);
			if (text.size() > 0)
				return text;
		}
	}
	return nullptr;
}

xr_string get_full_description(CHUDProfile* profile)
{
	if (profile && profile->description_fn.size()>0)
	{
		IReader* F = FS.r_open(profile->description_fn.c_str());
		if (F)
		{
			xr_string text;
			F->r_string(text);
			xr_string all_text;
			while(!F->eof())
			{
				F->r_string(text);
				all_text += text;
				all_text += "\\n";
			}
			FS.r_close(F);
			if (all_text.size() > 0)
				return all_text;
		}
	}
	return nullptr;
}

luabind::object get_profiles(CMultiHUDs* _multiHUDs)
{
	luabind::object resulTable = luabind::newtable(ai().script_engine().lua());
	xr_vector<CHUDProfile>* profiles = _multiHUDs->GetProfiles();
	for (xr_vector<CHUDProfile>::iterator it = profiles->begin(); it != profiles->end(); ++it)
			resulTable[std::distance(profiles->begin(), it)] = *it;
	return resulTable;
}

xr_string get_preview_texture(CHUDProfile* profile)
{
	std::string temp(profile->folder_path.c_str());
	temp+= profile->preview_texture_fn.c_str();
	return temp.c_str();
	//return nullptr;
}

using namespace luabind;

void multihuds_registrator::script_register(lua_State *L)
{
	module(L)
		[
			class_<CMultiHUDs>("CMultiHUDs")
				.def("profiles", &get_profiles)
				.def("current",&CMultiHUDs::GetCurrentProfile)
				.def("enabled", &CMultiHUDs::EnabledMultiHUDs)
				.def("ReloadHUD", &CMultiHUDs::HUDChanged),
			class_<CHUDProfile>("CHUDProfile")
			.def("get_name",&CHUDProfile::GetProfileName)
			.def("get_short_description",&get_short_description)
			.def("get_full_description", &get_full_description)
			.def("get_folder",&get_folder)
			.def("get_preview_texture", &get_preview_texture),
			def("get_multihuds", get_multihuds)
		];
}