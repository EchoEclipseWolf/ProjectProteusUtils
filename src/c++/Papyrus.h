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
				for (int i = 0; i < a_actor->standingStonePerks.size(); ++i) {
					const auto incAmt = static_cast<uint64_t>(0x8) * static_cast<uint64_t>(i * 2);
					const auto list = *(uint64_t*)(reinterpret_cast<uint64_t>(a_actor) + static_cast<uint64_t>(0x4E0));
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
			

			return result;
		}

		

		static inline bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("GetStandingStonePerkCount", CLASS_NAME, GetStandingStonePerkCount);
			a_vm->RegisterFunction("GetCurrentStandingStonePerk", CLASS_NAME, GetCurrentStandingStonePerk);
			a_vm->RegisterFunction("GetAllPerks", CLASS_NAME, GetAllPerks);
			a_vm->RegisterFunction("GetAllVisiblePerks", CLASS_NAME, GetAllVisiblePerks);

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
