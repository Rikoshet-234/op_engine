#ifndef lua_functionsH
#define lua_functionsH

#include "../pch_script.h"
#include "../ui/UIStatic.h"
#include "../Weapon.h"

namespace OPFuncs {
	void registerLuaOPFuncs(lua_State *L);
	bool luaFunctionExist(lua_State *L,std::string functionPath);
	CUIStatic* GetWeaponStatic(CWeapon weapon);
}

#endif