#include "stdafx.h"
#include "CMultiHUDs.h"
#include "../xrXMLParser/xrXMLParser.h"


ENGINE_API CMultiHUDs* multiHUDs = nullptr;
xr_string checkFile(xr_string pathName,xr_string fileName)
{
	string_path checkName;
	FS.update_path(checkName, "$game_huds$", pathName.c_str());
	strcat(checkName, fileName.c_str());
	if (FS.exist(checkName))
	{
		return checkName;
	}
	return xr_string();
}

LPCSTR HUDProfile::GetProfileConfigUIPath()
{
	string512 buf;
	sprintf_s(buf, "%sconfig\\ui", folder_path.c_str());
	return xr_strdup(buf);
}

LPCSTR HUDProfile::GetProfilePath()
{
	string_path path;
	FS.update_path(path, HUDS_PATH, folder_path.c_str());
	return xr_strdup(path);
}

bool HUDProfile::ExistFileInProfile(LPCSTR fileName)
{
	auto findResult = std::find_if(files.begin(), files.end(), [&](shared_str name)
	{
		return std::string(name.c_str()).find(fileName)!=std::string::npos;
	});
	return findResult != files.end();
}

CMultiHUDs::CMultiHUDs()
{
	psCurrentHUDProfileIndex=static_cast<u32>(-1);
	FS_FileSet				flist;
	FS.file_list(flist, HUDS_PATH, FS_ListFolders| FS_RootOnly);
	std::for_each(flist.begin(), flist.end(), [&](FS_File file)
	{
		if (checkFile(file.name, "config\\ui\\maingame.xml").size() > 0)
		{
			hudProfiles.push_back(HUDProfile());
			HUDProfile* profile = &hudProfiles.back();
			profile->folder_path = file.name.c_str();
			profile->description_fn = checkFile(profile->folder_path.c_str(), "description.txt").c_str();
			profile->preview_texture_fn = checkFile(profile->folder_path.c_str(), "preview.seq").c_str();
			FS_FileSet profileFiles;
			string_path profilePath;
			FS.update_path(profilePath, HUDS_PATH, profile->folder_path.c_str());
			FS.file_list(profileFiles, profilePath, FS_ListFiles);
			std::for_each(profileFiles.begin(), profileFiles.end(), [&](FS_File pFile)
			{
				profile->files.push_back(pFile.name.c_str());
			});
		}
	});
	if (hudProfiles.size() > 0)
	{
		auto defProfile = std::find_if(hudProfiles.begin(), hudProfiles.end(), [](HUDProfile profile)
		{
			return xr_strcmp(profile.folder_path.c_str(), "default\\") == 0;
		});
		if (defProfile == hudProfiles.end())
		{
			Msg("! Multi HUDs found, but profile 'default' not found!");
		}
		else
			psCurrentHUDProfileIndex = std::distance(hudProfiles.begin(), defProfile);
		tokens.clear();
		std::for_each(hudProfiles.begin(), hudProfiles.end(), [&](HUDProfile profile)
		{
			IReader* F = FS.r_open(profile.description_fn.c_str());
			if (F)
			{
				xr_string test;
				F->r_string(test);
				FS.r_close(F);
				tokens.push_back(xr_token());
				xr_token* last = &tokens.back();
				last->name = xr_strdup(test.c_str());
				last->id = tokens.size() - 1;
			}
		});
	}
}

bool CMultiHUDs::EnabledMultiHUDs() const
{
	return psCurrentHUDProfileIndex != static_cast<u32>(-1) && hudProfiles.size() > 0;
}

HUDProfile* CMultiHUDs::GetCurrentProfile()
{
	if (psCurrentHUDProfileIndex==static_cast<u32>(-1) || psCurrentHUDProfileIndex>hudProfiles.size())
		return nullptr;
	return &hudProfiles[psCurrentHUDProfileIndex];
}
