#pragma once

class ENGINE_API HUDProfile
{
public:
	bool ExistFileInProfile(LPCSTR fileName);
	LPCSTR GetProfilePath();
	LPCSTR GetProfileConfigUIPath();
	xr_vector<shared_str> files;
	shared_str folder_path;
	shared_str description_fn;
	shared_str preview_texture_fn;
};

class ENGINE_API CMultiHUDs
{
private:
	xr_vector<HUDProfile> hudProfiles;
public:
	xr_vector<xr_token> tokens;
	u32	psCurrentHUDProfileIndex;
	CMultiHUDs();

	bool EnabledMultiHUDs() const;
	HUDProfile* GetCurrentProfile();
};

extern ENGINE_API	CMultiHUDs*	multiHUDs;
