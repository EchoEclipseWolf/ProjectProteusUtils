#pragma once

#include "RE/Skyrim.h"
#include "Papyrus/Functions/Perks.h"
#include "Papyrus/Functions/Spells.h"
#include "Papyrus/Functions/Items.h"

namespace Papyrus
{
	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		Perk::Bind(*a_vm);
		Spell::Bind(*a_vm);
		Item::Bind(*a_vm);

		return true;
	}
}
