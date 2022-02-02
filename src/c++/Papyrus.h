#pragma once

#include "RE/Skyrim.h"

namespace Papyrus
{
	class ProteusDLLUtils
	{
	public:

		static inline std::int32_t GetStandingStonePerkCount(RE::StaticFunctionTag*, const RE::PlayerCharacter* a_actor)
		{
			if (a_actor != nullptr) {
				return a_actor->standingStonePerks.size();
			}
			return 0;
		}

		static inline RE::BGSPerk* GetCurrentStandingStonePerk(RE::StaticFunctionTag*, const RE::PlayerCharacter* a_actor)
		{
			if (const auto player = RE::PlayerCharacter::GetSingleton(); player == a_actor) {
				for (uint32_t i = 0; i < a_actor->standingStonePerks.size(); ++i) {
					const auto incAmt = static_cast<uint64_t>(0x8) * static_cast<uint64_t>(i * 2);
					const auto list = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint64_t>(a_actor) + static_cast<uint64_t>(0x4E0));
					const auto currentPerkAddress = *reinterpret_cast<uint64_t*>(list + incAmt);
					const auto currentPerk = reinterpret_cast<RE::BGSPerk*>(currentPerkAddress);
					if (currentPerk != nullptr) {
						const auto name = currentPerk->fullName;
						if (!currentPerk->data.hidden) {
							return currentPerk;
						}
					}
				}
			}
			return nullptr;
		}

		static inline auto GetAllPerks(RE::StaticFunctionTag*, const RE::Actor* a_actor) {
			std::vector<RE::BGSPerk*> result;

			if (!a_actor) {
				return result;
			}

			if (const auto player = RE::PlayerCharacter::GetSingleton(); player == a_actor) {
				if (const auto perkArray = player->addedPerks; !perkArray.empty()) {
					for (const auto perkData : perkArray) {
						if (const auto perk = perkData->perk; perk) {
							result.emplace_back(perk);
						}
					}
				}
			} else {
				if (const auto actorBase = a_actor->GetActorBase(); actorBase) {
					if (const auto perkArray = actorBase->As<RE::BGSPerkRankArray>(); perkArray) {
						for (std::uint32_t i = 0; i < perkArray->perkCount; ++i) {
							const auto perkData = perkArray->perks[i];

							if (const auto perk = perkData.perk; perk) {
								result.emplace_back(perk);
							}
						}
					}
				}
			}

			logger::info("Found {} perks", result.size());
			return result;
		}

		static inline auto GetAllVisiblePerks(RE::StaticFunctionTag* tag, const RE::Actor* a_actor)
		{
			std::vector<RE::BGSPerk*> result;

			for (const auto perks = GetAllPerks(tag, a_actor); auto perk : perks) {
				if (!perk->data.hidden) {
					result.emplace_back(perk);
				}
			}
			
			logger::info("Found {} visible perks", result.size());
			return result;
		}


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
			const auto raceSpellCount = race->actorEffects->numSpells;
			for (uint32_t i = 0; i < raceSpellCount; ++i) {
				const auto spell = race->actorEffects->spells[i];
				result.emplace_back(spell);
			}

			const RE::TESNPC* actorBase = a_actor->GetActorBase();
			const auto actorBaseSpellCount = actorBase->actorEffects->numSpells;
			for (uint32_t i = 0; i < actorBaseSpellCount; ++i) {
				const auto spell = actorBase->actorEffects->spells[i];
				result.emplace_back(spell);
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

		

		static inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("GetStandingStonePerkCount", CLASS_NAME, GetStandingStonePerkCount);
			a_vm->RegisterFunction("GetCurrentStandingStonePerk", CLASS_NAME, GetCurrentStandingStonePerk);
			a_vm->RegisterFunction("GetAllPerks", CLASS_NAME, GetAllPerks);
			a_vm->RegisterFunction("GetAllVisiblePerks", CLASS_NAME, GetAllVisiblePerks);
			a_vm->RegisterFunction("GetAllSpells", CLASS_NAME, GetAllSpells);
			a_vm->RegisterFunction("GetAllFavoritedSpells", CLASS_NAME, GetAllFavoritedSpells);
			a_vm->RegisterFunction("GetAllShouts", CLASS_NAME, GetAllShouts);
			a_vm->RegisterFunction("GetAllFavoritedItems", CLASS_NAME, GetAllFavoritedItems);

			logger::info("Registered funcs for class {}", CLASS_NAME);

			return true;
		}

	private:
		static constexpr char CLASS_NAME[] = "ProteusDLLUtils";
	};

	inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		ProteusDLLUtils::Register(a_vm);
		return true;
	}
}
