#pragma once

class CExtraContentFilter
{

	struct PackData
	{
		shared_str				sPackName;
		bool					bEnabled;
		xr_vector<shared_str>	aContent;
	};

	xr_vector<PackData*>		aDataPacks;
public:
				CExtraContentFilter				();
	virtual		~CExtraContentFilter			();

		bool	IsDataEnabled(LPCSTR pData);		
};