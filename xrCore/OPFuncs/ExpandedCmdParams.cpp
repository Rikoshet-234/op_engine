#include "stdafx.h"

#include "ExpandedCmdParams.h"


namespace OPFuncs
{
	XRCORE_API ExpandedCmdParams* Dumper=new ExpandedCmdParams();
	
	ExpandedCmdParams::ExpandedCmdParams()
	{
		knownCmdParams = createMap<std::string,int>
		("-dumpthm",KnownParams::dTHMLoad)
		("-dumpfsv",KnownParams::dFileSystemVars)
		("-dumpmtr",KnownParams::dMaterialsLoad)
		("-dumptl",KnownParams::dTextureLoad)
		("-dumpslp",KnownParams::dScriptsLoad)
		("-dumpsrl",KnownParams::dSpawnRegistryLoad)
		("-dumpcsr",KnownParams::dClassScriptRegister)
		("-dumpaor",KnownParams::dAlifeObjectRegister)
		("-dumpml",KnownParams::dMapLoad)
		("-dumpmle",KnownParams::dMapLoadErrors)
		("-dumpfs",KnownParams::dFileSystem)
		("-dumpspnp",KnownParams::dSoundPrefixNotPresent)
		("-slt",KnownParams::pShowLogTime);
	}
	
	void ExpandedCmdParams::ParseCmdParams(std::string cmdLine)
	{
		paramFlags.zero();
		bool dumpAll=false;
		if (cmdLine.find("-dumpall")!= std::string::npos)
		{
			dumpAll=true;
		} 
		for (ParamsMap::iterator it = knownCmdParams.begin(); it != knownCmdParams.end(); ++it)
		{
			if (dumpAll || cmdLine.find(it->first)!= std::string::npos)
			{
				paramFlags.set(it->second,TRUE);
			}
		}
	}
	
	bool ExpandedCmdParams::isParamSet(int param) const
	{
		return paramFlags.is(param)==TRUE;
	}

	bool ExpandedCmdParams::isAnyParamsSet(int param) const
	{
		return paramFlags.is_any(param)==TRUE;
	}

}
