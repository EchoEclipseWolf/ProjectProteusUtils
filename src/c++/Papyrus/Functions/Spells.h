#pragma once

namespace Papyrus::Spell
{
	inline auto GetAllSpells(RE::StaticFunctionTag*, const RE::Actor* a_actor)
	{
		std::vector<RE::SpellItem*> result;
		if (a_actor == nullptr) {
			return result;
		}

		for (auto spell : a_actor->GetActorRuntimeData().addedSpells) {
			result.emplace_back(spell);
		}

		const auto race = a_actor->GetActorRuntimeData().race;
		if (race != nullptr && race->actorEffects != nullptr && race->actorEffects->numSpells > 0) {
			const auto raceSpellCount = race->actorEffects->numSpells;
			for (uint32_t i = 0; i < raceSpellCount; ++i) {
				const auto spell = race->actorEffects->spells[i];
				result.emplace_back(spell);
			}
		}

		const RE::TESNPC* actorBase = a_actor->GetActorBase();
		if (actorBase != nullptr && actorBase->actorEffects != nullptr && actorBase->actorEffects->numSpells > 0) {
			const auto actorBaseSpellCount = actorBase->actorEffects->numSpells;
			for (uint32_t i = 0; i < actorBaseSpellCount; ++i) {
				const auto spell = actorBase->actorEffects->spells[i];
				result.emplace_back(spell);
			}
		}

		logger::info("Found {} Spells", result.size());
		return result;
	}

	inline auto IsFavoritedSpell(RE::StaticFunctionTag* tag, RE::SpellItem* spell)
	{
		const auto magicFavorites = RE::MagicFavorites::GetSingleton();
		if (magicFavorites == nullptr) {
			return false;
		}

		for (const auto favoritedSpell : magicFavorites->spells) {
			if (spell == favoritedSpell) {
				return true;
			}
		}

		return false;
	}

	inline auto GetAllFavoritedSpells(RE::StaticFunctionTag* tag)
	{
		const auto start = std::chrono::steady_clock::now();

		std::vector<RE::SpellItem*> result;

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (player == nullptr) {
			return result;
		}

		if (RE::MagicFavorites::GetSingleton() == nullptr) {
			return result;
		}

		for (const std::vector<RE::SpellItem*> spells = GetAllSpells(tag, player); auto spell : spells) {
			if (IsFavoritedSpell(tag, spell)) {
				result.emplace_back(spell);
			}
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Found {} Favorited Spells", result.size());
		logger::info("		Favorited Spells Time: {}", diff.count());
		return result;
	}

	inline auto RemoveAllSpells(RE::StaticFunctionTag* tag, RE::Actor* a_actor)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		for (const auto allSpells = GetAllSpells(tag, a_actor); RE::SpellItem * spell : allSpells) {
			a_actor->RemoveSpell(spell);
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all spells on actor {}", a_actor->GetName());
		logger::info("		Removed Spells Time: {}", diff.count());
		return 1;
	}

	inline auto GetAllShouts(RE::StaticFunctionTag*, const RE::Actor* a_actor)
	{
		std::vector<RE::TESShout*> result;
		if (a_actor == nullptr) {
			return result;
		}

		const auto race = a_actor->GetActorRuntimeData().race;
		const auto raceSpellCount = race->actorEffects->numShouts;
		for (uint32_t i = 0; i < raceSpellCount; ++i) {
			const auto shout = race->actorEffects->shouts[i];
			result.emplace_back(shout);
		}

		const RE::TESNPC* actorBase = a_actor->GetActorBase();
		const auto actorBaseShoutCount = actorBase->actorEffects->numShouts;
		for (uint32_t i = 0; i < actorBaseShoutCount; ++i) {
			const auto shout = actorBase->actorEffects->shouts[i];
			const auto name = shout->fullName;
			logger::info("Found shout: {}", name);
			const auto known = shout->GetKnown();
			logger::info("		is known: {}", known);
			result.emplace_back(shout);
		}

		logger::info("Found {} shouts", result.size());
		return result;
	}

	inline auto SetShoutAsKnown(RE::StaticFunctionTag*, const RE::TESShout* shout, bool shouldKnow)
	{
		if (shout == nullptr) {
			return 0;
		}

		
		

		//logger::info("Found {} shouts", shout);
		return 1;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(GetAllSpells);
		BIND(IsFavoritedSpell);
		BIND(GetAllFavoritedSpells);
		BIND(RemoveAllSpells);
		BIND(GetAllShouts);

		logger::info("Registered Proteus Spell functions");
	}
}
