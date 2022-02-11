#pragma once
#include "Papyrus/Game/Graphics.h"
#include "RE/S/Sexes.h"

namespace Papyrus::Actor
{
	inline void ExecuteCommand(RE::StaticFunctionTag*, std::string a_command, RE::TESObjectREFR* a_reference)
	{
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		if (const auto script = scriptFactory ? scriptFactory->Create() : nullptr) {
			script->SetCommand(a_command);
			script->CompileAndRun(a_reference);
			delete script;
		}
	}

	inline auto SetSex(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor, int sex)
	{
		if (a_actor == nullptr) {
			return 0;
		}

		const auto npc = a_actor->GetActorBase();
		if (npc == nullptr) {
			return 0;
		}

		if (const auto currentSex = npc->GetSex(); currentSex == static_cast<RE::SEXES::SEX>(sex)) {
			return 1;
		}

		ExecuteCommand(tag, "SexChange", a_actor);
		logger::info("Set sex to: " + std::to_string(sex));
		return 1;
	}

	inline auto SetLevel(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag, RE::Actor* a_actor, int level)
	{
		if (a_actor == nullptr) {
			return 0;
		}

		ExecuteCommand(tag, "setlevel " + std::to_string(level), a_actor);
		logger::info("Set level to: " + std::to_string(level));
		return 1;
	}

	inline auto ShowRaceMenu(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag)
	{
		ExecuteCommand(tag, "showracemenu", nullptr);
		logger::info("Showed Race Menu");
		return 1;
	}

	inline auto ProteusSetHairColor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::Actor* a_actor,
		RE::BGSColorForm* a_color)
	{
		if (!a_actor) {
			logger::info("SetHairColor: Invalid Actor");
			return 0;
		}
		if (!a_color) {
			logger::info("SetHairColor: Invalid Colorform");
			return 0;
		}

		const auto root = a_actor->Get3D(false);
		if (!root) {
			logger::info("SetHairColor: Invalid 3D");
			return 0;
		}

		auto task = SKSE::GetTaskInterface();
		task->AddTask([root, a_actor, a_color]() {
			root->UpdateHairColor(a_color->color);

			if (const auto& biped = a_actor->GetCurrentBiped(); biped) {
				for (auto& slot : ACTOR::headSlots) {
					const auto node = biped->objects[slot].partClone;
					if (node && node->HasShaderType(RE::BSShaderMaterial::Feature::kHairTint)) {
						node->UpdateHairColor(a_color->color);
					}
				}
			}

			SET::update_color_data(root, EXTRA::HAIR_TINT, a_color->color);
		});

		return 1;
	}

	inline auto ProteusGetAllFactions(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		//std::set<RE::TESFaction*> factionRankSet;
		//const GameVisitors::CollectUniqueFactions factionVisitor(&factionRankSet, 127, -128);
		//a_actor->VisitFactions(factionVisitor);

		std::vector<RE::TESFaction*> result;

		const auto npc = a_actor->GetActorBase();
		if (npc == nullptr) {
			return result;
		}

		const auto factions = npc->factions;
		const auto factionCount = npc->factions.size();
		for (int i = 0; i < factionCount; ++i) {
			const RE::FACTION_RANK factionRank = factions[i];
			RE::TESFaction* faction = factionRank.faction;
			result.push_back(faction);
			logger::info("		Faction {}: {}", faction->fullName, factionRank.rank);
		}

		logger::info("Found {} factions", factionCount);
		return result;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(SetSex);
		BIND(SetLevel);
		BIND(ShowRaceMenu);
		BIND(ProteusSetHairColor);
		BIND(ProteusGetAllFactions);

		logger::info("Registered Proteus Actor functions");
	}
}
