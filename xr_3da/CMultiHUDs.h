#ifndef CMultiHUDs_h
#define CMultiHUDs_h
#pragma once

class ENGINE_API CHUDProfile
{
private:
	xr_string CHUDProfile::createFileName(xr_string oldFileName, xr_string postfix);
public:
	xr_string ExistFileInProfile(LPCSTR fileName);
	xr_string GetProfileName();
	xr_string GetFileFromProfile(LPCSTR fileName, bool remExt=false);
	xr_string GetProfileConfigUIPath();
	xr_string GetProfileConfigPath();
	xr_vector<shared_str> files;
	shared_str folder_path;
	shared_str description_fn;
	shared_str preview_texture_fn;
};

class ENGINE_API CMultiHUDs
{
private:
	xr_vector<CHUDProfile> hudProfiles;
public:
	xr_vector<xr_token> tokens;
	u32	psCurrentHUDProfileIndex;
	CMultiHUDs();
	~CMultiHUDs();
	bool EnabledMultiHUDs() const;
	CHUDProfile* GetCurrentProfile();
	CHUDProfile* GetProfile(LPCSTR profileName);
	xr_vector<CHUDProfile> *GetProfiles() { return &hudProfiles; }

	void HUDChanged();
};

extern ENGINE_API	CMultiHUDs*	multiHUDs;

#endif