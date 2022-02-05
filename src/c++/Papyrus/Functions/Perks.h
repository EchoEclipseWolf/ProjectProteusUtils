#pragma once

namespace Papyrus::Perk
{
	inline RE::BGSPerk* GetCurrentStandingStonePerk(RE::StaticFunctionTag*, const RE::PlayerCharacter* a_actor)
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

	inline auto GetAllPerks(RE::StaticFunctionTag*, const RE::Actor* a_actor)
	{
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

		

	inline auto GetAllVisiblePerks(RE::StaticFunctionTag* tag, const RE::Actor* a_actor)
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

	inline auto RemoveAllVisiblePerks(RE::StaticFunctionTag* tag, RE::Actor* a_actor)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		for (const auto allPerks = GetAllVisiblePerks(tag, a_actor); auto perk : allPerks) {
			a_actor->RemovePerk(perk);
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all perks on actor {}", a_actor->GetName());
		logger::info("		Removed Perks Time: {}", diff.count());
		return 1;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(GetCurrentStandingStonePerk);
		BIND(GetAllPerks);
		BIND(GetAllVisiblePerks);
		BIND(RemoveAllVisiblePerks);

		logger::info("Registered Proteus Perk functions");
	}
}
