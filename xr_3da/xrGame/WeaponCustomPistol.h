#pragma once

#include "WeaponMagazined.h"

#define SND_RIC_COUNT 5
 
class CWeaponCustomPistol: public CWeaponMagazined
{
private:
	typedef CWeaponMagazined inherited;
public:
					CWeaponCustomPistol	(LPCSTR name);
	virtual			~CWeaponCustomPistol();

	int				GetCurrentFireMode	() override			{ return 1; };
protected:
	virtual void	FireEnd				();
	virtual void	switch2_Fire		();
};
