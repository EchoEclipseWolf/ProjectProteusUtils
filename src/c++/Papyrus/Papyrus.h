#pragma once

#include "RE/Skyrim.h"
#include "RE/A/Actor.h"
#include "Papyrus/Functions/Perks.h"

namespace Papyrus
{

	class ProteusDLLUtils
	{
	public:

		static inline auto GetAllSpells(RE::StaticFunctionTag*, const RE::Actor* a_actor)
		{
			std::vector<RE::SpellItem*> result;
			if (a_actor == nullptr) {
				return result;
			}

			for (auto spell : a_actor->addedSpells) {
				result.emplace_back(spell);
			}

			const auto race = a_actor->race;
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

		static inline auto IsFavoritedSpell(RE::SpellItem* spell)
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

		static inline auto GetAllFavoritedSpells(RE::StaticFunctionTag* tag)
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
				if (IsFavoritedSpell(spell)) {
					result.emplace_back(spell);
				}
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			
			logger::info("Found {} Favorited Spells", result.size());
			logger::info("		Favorited Spells Time: {}", diff.count());
			return result;
		}

		static inline auto RemoveAllSpells(RE::StaticFunctionTag* tag, RE::Actor* a_actor)
		{
			const auto start = std::chrono::steady_clock::now();

			if (a_actor == nullptr) {
				return 0;
			}

			for (const auto allSpells = GetAllSpells(tag, a_actor); RE::SpellItem* spell : allSpells) {
				a_actor->RemoveSpell(spell);
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			logger::info("Removed all spells on actor {}", a_actor->GetName());
			logger::info("		Removed Spells Time: {}", diff.count());
			return 1;
		}

		static inline auto GetAllShouts(RE::StaticFunctionTag*, const RE::Actor* a_actor)
		{
			std::vector<RE::TESShout*> result;
			if (a_actor == nullptr) {
				return result;
			}

			const auto race = a_actor->race;
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

		static inline auto GetAllFavoritedItems(RE::StaticFunctionTag* tag)
		{
			const auto start = std::chrono::steady_clock::now();

			std::vector<RE::TESBoundObject*> result;

			const auto player = RE::PlayerCharacter::GetSingleton();
			if (player == nullptr) {
				return result;
			}

			auto inv = player->GetInventory();
			int invCount = 0;
			for (const auto& [item, data] : inv) {
				if (item->Is(RE::FormType::LeveledItem)) {
					continue;
				}
				const auto& [count, entry] = data;
				invCount += count;
				if (count > 0 && entry->IsFavorited()) {
					result.push_back(item);
					logger::info("		Found Favorite Item: {}", entry->GetDisplayName());
				}
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			logger::info("Found {} Inventory Items", invCount);
			logger::info("Found {} Favorited Items", result.size());
			logger::info("		Favorited Items Time: {}", diff.count());
			return result;
		}

		static inline auto GetCombinedIndex(const RE::TESFile* a_file)
		{
			return static_cast<uint32_t>(a_file->compileIndex + a_file->smallFileCompileIndex);
		}

		static void AddForms(RE::FormType a_type)
		{
			if (_formCache.contains(a_type)) {
				return;
			}

			RE::FormType type = a_type;
			/* switch (a_type) {
			case RE::FormType::Spell:
				{
					type = RE::FormType::Book;
					break;
				}
			}*/

			const auto handler = RE::TESDataHandler::GetSingleton();
			if (handler == nullptr) {
				return;
			}
			const auto& items = handler->GetFormArray(type);
			
			_formCache[type] = items;
		}

		static inline auto ProteusGetItemCount(RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
		{
			const auto start = std::chrono::steady_clock::now();

			AddForms(formType);

			const auto strContains = std::string(containsName);

			if (_formCache.contains(formType)) {
				const auto items = _formCache[formType];
				int count = 0;
				for (const auto item : items) {
					if (item != nullptr) {
						const auto name = item->GetName();
						if (name != nullptr) {
							const auto strName = std::string(name);

							auto it = std::search(
								strName.begin(), strName.end(),
								strContains.begin(), strContains.end(),
								[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
							if (it != strName.end()) {
								count++;
							}
						}						
					}
				}

				const auto end = std::chrono::steady_clock::now();
				const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

				logger::info("Found {} items in the game", count);
				logger::info("		Total Found Time: {}", diff.count());
				return count;
			}

			return 0;
		}

		static inline auto ProteusGetItemBySearch(RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
		{
			const auto start = std::chrono::steady_clock::now();

			std::vector<RE::TESForm*> result;

			AddForms(formType);

			const auto strContains = std::string(containsName);

			if (_formCache.contains(formType)) {
				const auto items = _formCache[formType];
				int count = 0;
				for (const auto item : items) {
					if (item != nullptr) {
						const auto name = item->GetName();
						if (name != nullptr) {
							const auto strName = std::string(name);

							auto it = std::search(
								strName.begin(), strName.end(),
								strContains.begin(), strContains.end(),
								[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
							if (it != strName.end()) {
								result.emplace_back(item);
								count++;
							}
						}
					}
				}

				const auto end = std::chrono::steady_clock::now();
				const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

				logger::info("Found {} items in the game", count);
				logger::info("		Total Found Time: {}", diff.count());
			}

			return result;
		}

		static inline auto ProteusGetItemEditorIdBySearch(RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
		{
			const auto start = std::chrono::steady_clock::now();

			std::vector<RE::TESForm*> result;

			AddForms(formType);

			const auto strContains = std::string(containsName);

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			//logger::info("Found {} items in the game by editor id", count);
			logger::info("		Total Found Time: {}", diff.count());

			return result;
		}

		static inline auto ProteusGetAllByFormId(RE::StaticFunctionTag* tag, RE::FormType formType)
		{
			const auto start = std::chrono::steady_clock::now();

			std::vector<RE::TESForm*> result;

			AddForms(formType);

			if (_formCache.contains(formType)) {
				const auto items = _formCache[formType];
				int count = items.size();

				const auto end = std::chrono::steady_clock::now();
				const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

				logger::info("Found {} total form items", count);
				logger::info("		Total Found Time: {}", diff.count());
			}

			return result;
		}

		static inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("GetAllSpells", CLASS_NAME, GetAllSpells);
			a_vm->RegisterFunction("GetAllFavoritedSpells", CLASS_NAME, GetAllFavoritedSpells);
			a_vm->RegisterFunction("RemoveAllSpells", CLASS_NAME, RemoveAllSpells);
			a_vm->RegisterFunction("GetAllShouts", CLASS_NAME, GetAllShouts);
			a_vm->RegisterFunction("GetAllFavoritedItems", CLASS_NAME, GetAllFavoritedItems);
			a_vm->RegisterFunction("ProteusGetItemCount", CLASS_NAME, ProteusGetItemCount);
			a_vm->RegisterFunction("ProteusGetItemBySearch", CLASS_NAME, ProteusGetItemBySearch);
			a_vm->RegisterFunction("ProteusGetItemEditorIdBySearch", CLASS_NAME, ProteusGetItemEditorIdBySearch);
			a_vm->RegisterFunction("ProteusGetAllByFormId", CLASS_NAME, ProteusGetAllByFormId);

			logger::info("Registered funcs for class {}", CLASS_NAME);

			return true;
		}		

	private:
		static constexpr char CLASS_NAME[] = "ProteusDLLUtils";
		static inline std::map<RE::FormType, RE::BSTArray<RE::TESForm*>> _formCache;
		static inline bool _hasSetupPlugins = false;
	};

	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		Perk::Bind(*a_vm);

		return true;
	}
}
