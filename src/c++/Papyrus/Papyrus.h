#pragma once

#include "RE/Skyrim.h"
#include "Papyrus/Functions/Perks.h"
#include "Papyrus/Functions/Spells.h"
#include "Papyrus/Functions/Items.h"
#include "Papyrus/Functions/Outfit.h"
#include "Papyrus/Functions/Actor.h"
#include "Papyrus/Functions/Utility.h"
#include "Papyrus/Functions/Form.h"
#include "Papyrus/Functions/Quests.h"

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
		//Outfit::Bind(*a_vm);
		Actor::Bind(*a_vm);
		Utility::Bind(*a_vm);
		Form::Bind(*a_vm);
		Quests::Bind(*a_vm);

		return true;
	}
}
