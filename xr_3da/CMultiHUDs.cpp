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


LPCSTR CHUDProfile::GetProfileConfigUIPath()
{
	string512 buf;
	sprintf_s(buf, "%sconfig\\ui", folder_path.c_str());
	return xr_strdup(buf);
}

LPCSTR CHUDProfile::GetProfilePath()
{
	string_path path;
	FS.update_path(path, HUDS_PATH, folder_path.c_str());
	return xr_strdup(path);
}

bool CHUDProfile::ExistFileInProfile(LPCSTR fileName)
{
	return GetFileFromProfile(fileName) != nullptr;
}

xr_string CHUDProfile::GetProfileName()
{
	xr_string tmp = folder_path.c_str();
	auto it = std::remove_if(std::begin(tmp), std::end(tmp), [](char c) {return (c == '\\'); });
	tmp.erase(it, std::end(tmp));
	return tmp;
}

LPCSTR CHUDProfile::GetFileFromProfile(LPCSTR fileName,bool remExt)
{
	std::string temp1(fileName);
	std::transform(temp1.begin(), temp1.end(), temp1.begin(), ::tolower);
	auto findResult = std::find_if(files.begin(), files.end(), [&](shared_str name)
	{
		return std::string(name.c_str()).find(temp1) != std::string::npos;
	});
	if (findResult != files.end())
	{
		LPCSTR temp= xr_strdup((*findResult).c_str());
		if (remExt)
		{
			LPSTR _ext = strext(temp);
			if (_ext)
				*_ext = 0;
		}
		return temp;
	}
	return nullptr;
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
			hudProfiles.push_back(CHUDProfile());
			CHUDProfile* profile = &hudProfiles.back();
			profile->folder_path = file.name.c_str();
			profile->description_fn = checkFile(profile->folder_path.c_str(), "description.txt").c_str();
			string_path texture_path;
			sprintf_s(texture_path, "%spreview", profile->folder_path.c_str());
			//profile->preview_texture_fn = texture_path;
			profile->preview_texture_fn = "preview";//checkFile(profile->folder_path.c_str(), "preview.seq").c_str();
			FS_FileSet profileFiles;
			string_path profilePath;
			FS.update_path(profilePath, HUDS_PATH, profile->folder_path.c_str());
			FS.file_list(profileFiles, profilePath, FS_ListFiles);
			std::for_each(profileFiles.begin(), profileFiles.end(), [&](FS_File pFile)
			{
				string_path temp;
				strcpy_s(temp, profile->folder_path.c_str());
				strcat_s(temp, pFile.name.c_str());
				profile->files.push_back(temp);
			});
		}
	});
	if (hudProfiles.size() > 0)
	{
		auto defProfile = std::find_if(hudProfiles.begin(), hudProfiles.end(), [](CHUDProfile profile)
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
		std::for_each(hudProfiles.begin(), hudProfiles.end(), [&](CHUDProfile profile)
		{
			tokens.push_back(xr_token());
			xr_token* last = &tokens.back();
			//xr_string tmp = profile.folder_path.c_str();
			//auto it = std::remove_if(std::begin(tmp), std::end(tmp), [](char c) {return (c == '\\'); });
			//tmp.erase(it, std::end(tmp));
			last->name = xr_strdup(profile.GetProfileName().c_str());//xr_strdup(tmp.c_str());
			last->id = tokens.size() - 1;
		});
	}
}

bool CMultiHUDs::EnabledMultiHUDs() const
{
	return psCurrentHUDProfileIndex != static_cast<u32>(-1) && hudProfiles.size() > 0;
}

CHUDProfile* CMultiHUDs::GetCurrentProfile()
{
	return EnabledMultiHUDs() ? &hudProfiles[psCurrentHUDProfileIndex] : nullptr ;
}

CHUDProfile* CMultiHUDs::GetProfile(LPCSTR profileName)
{
	xr_vector<CHUDProfile>::iterator it=std::find_if(hudProfiles.begin(),hudProfiles.end(),[&](CHUDProfile profile)
	{
		return xr_strcmp(profileName, profile.GetProfileName().c_str()) == 0;
	});
	if (it!=hudProfiles.end())
		return &*it;
	return nullptr;
}

