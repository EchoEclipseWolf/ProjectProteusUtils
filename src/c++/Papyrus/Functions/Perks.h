#pragma once

namespace Papyrus::Perk
{
	inline RE::BGSPerk* GetCurrentStandingStonePerk(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::PlayerCharacter* a_actor)
	{
		if (const auto player = RE::PlayerCharacter::GetSingleton(); player == a_actor) {
			for (uint32_t i = 0; i < a_actor->GetPlayerRuntimeData().standingStonePerks.size(); ++i) {
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

	inline auto GetAllPerks(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::Actor* a_actor)
	{
		std::vector<RE::BGSPerk*> result;

		if (!a_actor) {
			return result;
		}

		if (a_actor->As<RE::PlayerCharacter>() != nullptr) {
			auto player = a_actor->As<RE::PlayerCharacter>();
			auto perkArray = player->GetPlayerRuntimeData().addedPerks;
			if ( !perkArray.empty()) {
				for (auto perkData : perkArray) {
					if (auto perk = perkData->perk; perk) {
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

	inline auto AddAndGetConnectionsForPerkTreeNode(std::map<uint64_t, RE::BGSSkillPerkTreeNode*>& map, RE::BGSSkillPerkTreeNode* node)
	{
		if (node == nullptr) {
			return;
		}

		if (const auto addr = reinterpret_cast<uint64_t>(node); !map.contains(addr)) {
			map[addr] = node;
		}

		std::queue<RE::BGSSkillPerkTreeNode*> childrenToParse;

		for (auto childNode : node->children) {
			childrenToParse.push(childNode);
		}

		while (!childrenToParse.empty()) {
			const auto childNode = childrenToParse.front();
			if (const auto addr = reinterpret_cast<uint64_t>(childNode); !map.contains(addr)) {
				map[addr] = childNode;
			}
			for (auto subChildNode : childNode->children) {
				if (const auto addr = reinterpret_cast<uint64_t>(subChildNode); !map.contains(addr)) {
					childrenToParse.push(subChildNode);
				}				
			}
			childrenToParse.pop();
		}

	}

	inline auto GetAllVisiblePerks(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, const RE::Actor* a_actor)
	{
		std::vector<RE::BGSPerk*> result;

		const auto avl = RE::ActorValueList::GetSingleton();
		if (avl == nullptr) {
			return result;
		}

		const auto av = avl->LookupActorValueByName("Illusion");
		const auto actorValueInfo = avl->GetActorValue(av);
		const auto perkTree = actorValueInfo->perkTree;

		std::map<uint64_t, RE::BGSSkillPerkTreeNode*> map;

		if (perkTree != nullptr) {
			AddAndGetConnectionsForPerkTreeNode(map, perkTree);
		}

		for (const auto &pair : map) {
			const auto perkTreeNode = pair.second;
			auto perk = perkTreeNode->perk;

			while (perk != nullptr) {
				if (a_actor->HasPerk(perk)) {
				}

				result.push_back(perk);
				perk = perk->nextPerk;
			}
			
		}

		logger::info("Found {} visible perks", result.size());
		return result;
	}

	inline auto RemoveAllPerks(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		int count = 0;
		for (const auto allPerks = GetAllPerks(a_vm, a_stackID, tag, a_actor); auto perk : allPerks) {
			a_actor->RemovePerk(perk);
			++count;
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all perks on actor {}", a_actor->GetName());
		logger::info("		Removed Perks Time: {}", diff.count());
		return count;
	}

	inline auto RemoveAllVisiblePerks(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		for (const auto allPerks = GetAllVisiblePerks(a_vm, a_stackID, tag, a_actor); auto perk : allPerks) {
			a_actor->RemovePerk(perk);
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all perks on actor {}", a_actor->GetName());
		logger::info("		Removed Perks Time: {}", diff.count());
		return 1;
	}

	inline auto GetPerksForPerkTree(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, const RE::Actor* a_actor, RE::BSFixedString treeName, bool purchasedPerksOnly)
	{
		std::vector<RE::BGSPerk*> result;

		const auto avl = RE::ActorValueList::GetSingleton();
		if (avl == nullptr) {
			logger::info("Found {} perk tree perks", result.size());
			return result;
		}

		const auto av = avl->LookupActorValueByName(treeName);
		const auto actorValueInfo = avl->GetActorValue(av);
		if (actorValueInfo == nullptr) {
			logger::info("Found {} perk tree perks", result.size());
			return result;
		}
		const auto perkTree = actorValueInfo->perkTree;
		if (perkTree == nullptr) {
			logger::info("Found {} perk tree perks", result.size());
			return result;
		}

		std::map<uint64_t, RE::BGSSkillPerkTreeNode*> map;
		AddAndGetConnectionsForPerkTreeNode(map, perkTree);
		

		for (const auto& pair : map) {
			const auto perkTreeNode = pair.second;
			auto perk = perkTreeNode->perk;

			while (perk != nullptr) {
				if (purchasedPerksOnly) {
					if (a_actor->HasPerk(perk)) {
						result.push_back(perk);
					}
				} else {
					result.push_back(perk);
				}
				
				perk = perk->nextPerk;
			}
		}

		logger::info("Found {} perk tree perks", result.size());
		return result;
	}

	inline auto GetAllPerkTreePerks(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, const RE::Actor* a_actor, bool purchasedPerksOnly)
	{
		std::vector<RE::BGSPerk*> result;

		const auto avl = RE::ActorValueList::GetSingleton();
		if (avl == nullptr) {
			logger::info("Found {} perk tree perks", result.size());
			return result;
		}

		for (int i = 0; i < static_cast<int>(RE::ActorValue::kTotal); ++i) {
			const auto actorValueInfo = avl->GetActorValue(static_cast<RE::ActorValue>(i));
			if (actorValueInfo == nullptr) {
				continue;
			}
			const auto perkTree = actorValueInfo->perkTree;
			if (perkTree == nullptr) {
				continue;
			}

			std::map<uint64_t, RE::BGSSkillPerkTreeNode*> map;

			AddAndGetConnectionsForPerkTreeNode(map, perkTree);
			

			for (const auto& pair : map) {
				const auto perkTreeNode = pair.second;
				auto perk = perkTreeNode->perk;

				while (perk != nullptr) {
					if (purchasedPerksOnly) {
						if (a_actor->HasPerk(perk)) {
							result.push_back(perk);
						}
					} else {
						result.push_back(perk);
					}
					perk = perk->nextPerk;
				}
			}
		}
		

		logger::info("Found {} perk tree perks", result.size());
		return result;
	}

	inline auto RemovePerksForTree(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor, RE::BSFixedString treeName)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		int removedCount = 0;
		const auto perksForTree = GetPerksForPerkTree(a_vm, a_stackID, tag, a_actor, treeName, true);
		for (auto perk : perksForTree) {
			a_actor->RemovePerk(perk);
			++removedCount;
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all perks for tree {} on actor {}", treeName, a_actor->GetName());
		logger::info("		Removed Perks Time: {}", diff.count());
		return removedCount;
	}

	inline auto RemovePerksForAllTrees(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor)
	{
		const auto start = std::chrono::steady_clock::now();

		if (a_actor == nullptr) {
			return 0;
		}

		int removedCount = 0;

		const auto perksForTree = GetAllPerkTreePerks(a_vm, a_stackID, tag, a_actor, true);
		for (auto perk : perksForTree) {
			a_actor->RemovePerk(perk);
			++removedCount;
		}

		const auto end = std::chrono::steady_clock::now();
		const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		logger::info("Removed all perks for all trees on actor {}", a_actor->GetName());
		logger::info("		Removed Perks Time: {}", diff.count());
		return removedCount;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(GetCurrentStandingStonePerk);
		BIND(GetAllPerks);
		BIND(GetAllVisiblePerks);
		BIND(RemoveAllPerks);
		BIND(RemoveAllVisiblePerks);
		BIND(GetPerksForPerkTree);
		BIND(GetAllPerkTreePerks);
		BIND(RemovePerksForTree);
		BIND(RemovePerksForAllTrees);

		logger::info("Registered Proteus Perk functions");
	}
}
