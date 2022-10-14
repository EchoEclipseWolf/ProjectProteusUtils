#pragma once
#include "Items.h"
#include <ShlObj.h>
#include<fstream>

#include "BinaryTools/BinaryReader.h"
#include "BinaryTools/BinaryWriter.h"
#include "Papyrus/Game/DataMap.h"

#include <string>
#include <iostream>
#include <assert.h>

namespace Papyrus::Quests
{
	static std::vector<RE::TESQuest*> allQuests;
	static std::unordered_map<std::string, RE::TESQuest*> cachedQuests;

	inline std::string utf8_encode(const std::wstring& wstr)
	{
		if (wstr.empty())
			return std::string();
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}

	

	inline auto FindHighestQuestStage(RE::TESQuest* quest)
	{
		if (quest->waitingStages == nullptr) {
			return static_cast<uint16_t>(0);
		}

		auto stages = *quest->waitingStages;
		const auto t = stages.front();

		uint16_t highestIndex = 0;


		for (auto it = stages.begin(); it != stages.end(); ++it) {
			const auto questStage = *it;
			const uint16_t index = questStage->data.index;
			const uint8_t flags = static_cast<uint8_t>(questStage->data.flags.get());

			if (flags > 0 && index > highestIndex) {
				highestIndex = index;
			}
		}
		return highestIndex;
	}

	inline auto LoadQuestStages(BinaryReader& rf, RE::TESQuest* quest)
	{
		if (quest == nullptr) {

			const auto waitingStagesCount = rf.ReadUint8();
			for (uint16_t i = 0; i < waitingStagesCount; ++i) {
				rf.ReadUint16();
				rf.ReadUint8();
			}

			return;
		}

		const auto stagesAddress = reinterpret_cast<uintptr_t*>(&quest->executedStages);
		const auto firstQuestStage = reinterpret_cast<RE::TESQuestStage*>(*stagesAddress);
		const uint8_t loadQuestStages = rf.ReadUint8();

		if (loadQuestStages > 0 && firstQuestStage == nullptr) {
			int bob = 1;
		}

		if (loadQuestStages == 0 || firstQuestStage == nullptr) {
			return;
		}

		for (int i = 0; i < loadQuestStages; ++i) {
			const auto index = rf.ReadUint16();
			const auto newFlag = rf.ReadUint8();


			auto stageContainer = reinterpret_cast<uintptr_t*>(&quest->executedStages);
			auto stage = reinterpret_cast<RE::TESQuestStage*>(*stageContainer);
			while (true) {
				stage = reinterpret_cast<RE::TESQuestStage*>(*stageContainer);

				if (stage == nullptr) {
					return; //There was a problem as we shouldnt get here
				}
				if (stage != nullptr) {
					if (stage->data.index == index) {
						break;
					}
				}

				stageContainer = reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(stageContainer) + static_cast<uintptr_t>(0x8)));
			}

			if (stage != nullptr) {
				const auto stageAddress = reinterpret_cast<uintptr_t>(stage);
				const auto flag = reinterpret_cast<uint8_t*>(stageAddress + 0x2);
				if (newFlag != 0) {
					*flag |= 1;
				} else {
					*flag &= 0xFE;
				}
			}
		}

		return;

		firstQuestStage->data.index = rf.ReadUint16();
		uint8_t firstFlag = 0;
		firstFlag = rf.ReadUint8();
		firstQuestStage->data.flags = static_cast<RE::QUEST_STAGE_DATA::Flag>(firstFlag);

		uint16_t count = 0;

		if (loadQuestStages > 0 && quest->waitingStages != nullptr) {
			auto stages = *quest->waitingStages;

			for (auto it = stages.begin(); it != stages.end(); ++it) {
				const auto questStage = *it;

				questStage->data.index = rf.ReadUint16();
				uint8_t flag = 0;
				flag = rf.ReadUint8();
				questStage->data.flags = static_cast<RE::QUEST_STAGE_DATA::Flag>(flag);

				count++;
				if (count >= loadQuestStages) {
					int bob = 1;  //Something went wrong
					break;
				}
			}
		}
	}

	inline auto SaveQuestStages(BinaryWriter& wf, RE::TESQuest* quest)
	{
		const auto stagesAddress = reinterpret_cast<uintptr_t*>(&quest->executedStages);
		const auto firstQuestStage = reinterpret_cast<RE::TESQuestStage*>(*stagesAddress);

		if (firstQuestStage == nullptr) {
			wf.WriteUint8(0);
			return;
		}

		uint8_t count = 1;
		if (quest->waitingStages != nullptr) {
			auto stages = *quest->waitingStages;

			for (auto it = stages.begin(); it != stages.end(); ++it) {
				++count;
			}
		}

		wf.WriteUint8(count);
		const uint16_t firstIndex = firstQuestStage->data.index;
		const uint8_t firstFlags = static_cast<uint8_t>(firstQuestStage->data.flags.get());

		wf.WriteUint16(firstIndex);
		wf.WriteUint8(firstFlags);

		if (quest->waitingStages != nullptr) {
			auto stages = *quest->waitingStages;

			for (auto it = stages.begin(); it != stages.end(); ++it) {
				const auto questStage = *it;
				const uint16_t index = questStage->data.index;
				const uint8_t flags = static_cast<uint8_t>(questStage->data.flags.get());
				const auto test = flags & 1;
				if (test > 0) {
					int bob = 1;
				}
				wf.WriteUint16(index);
				wf.WriteUint8(flags & 1);
			}
		}
	}

	inline auto LoadObjectives(BinaryReader& rf, RE::TESQuest* quest)
	{
		const uint16_t totalCount = rf.ReadUint16();

		if (totalCount == 0) {
			return;
		}

		if (quest == nullptr) {
			for (int i = 0; i < totalCount; ++i) {
				rf.ReadUint16();
				rf.ReadUint8();
			}

			return;
		}

		auto objectives = quest->objectives;
		int count = 0;
		for (auto it = objectives.begin(); it != objectives.end(); ++it) {
			const auto objective = *it;
			if (objective != nullptr) {
				objective->index = rf.ReadUint16();
				uint8_t state = 0;
				state = rf.ReadUint8();
				objective->state = static_cast<RE::QUEST_OBJECTIVE_STATE>(state);
			}
			count++;
			if (count >= totalCount) {
				break;
			}
		}

		while (count < totalCount) {
			rf.ReadUint16();
			rf.ReadUint8();
			count++;
		}
	}

	inline auto SaveObjectives(BinaryWriter& wf, RE::TESQuest* quest)
	{
		auto objectives = quest->objectives;
		uint16_t count = 0;

		for (auto it = objectives.begin(); it != objectives.end(); ++it) {
			++count;
		}

		wf.WriteUint16(count);

		for (auto it = objectives.begin(); it != objectives.end(); ++it) {
			const auto objective = *it;
			if (objective != nullptr) {
				const uint16_t index = objective->index;
				const uint8_t state = static_cast<uint8_t>(objective->state.get());
				wf.WriteUint16(index);
				wf.WriteUint8(state);
			}
		}
	}

	inline auto SaveInstanceData(BinaryWriter& wf, RE::TESQuest* quest)
	{
		wf.WriteUint32(quest->currentInstanceID);

		wf.WriteUint32(quest->instanceData.size());
		for (const auto instanceData : quest->instanceData) {
			wf.WriteUint32(instanceData->id);
			wf.WriteUint32(instanceData->stringData.size());

			for (const auto stringData : instanceData->stringData) {
				wf.WriteUint32(stringData.aliasID);
				wf.WriteUint32(stringData.fullNameFormID);
			}

			wf.WriteUint32(instanceData->valueData.size());
			for (const auto valueData : instanceData->valueData) {
				const auto globalEditorId = std::string(valueData.global->formEditorID);
				wf.WriteUint64(globalEditorId.size());
				wf.WriteFixedLengthString(globalEditorId);
				wf.WriteFloat(valueData.value);
			}

			wf.WriteUint16(instanceData->journalStage);
			wf.WriteInt8(instanceData->journalStageItem);
		}
	}

	inline RE::TESGlobal* FindGlobalForm(const std::string &editorId)
	{
		for (const auto form : Item::ProteusGetAllByFormId(nullptr, RE::FormType::Global)) {
			if (form != nullptr) {
				const auto globalForm = static_cast<RE::TESGlobal*>(form);
				const auto formEditorId = std::string(globalForm->formEditorID);
				if (formEditorId == editorId) {
					return globalForm;
				}
			}
		}

		return nullptr;
	}

	inline void AllocInstanceArray2(uintptr_t bstarray, int size, int a6)
	{
		using func_t = decltype(&AllocInstanceArray2);
#if ANNIVERSARY_EDITION
		REL::Relocation<func_t> func{ REL::ID(66913) };

#else
		REL::Relocation<func_t> func{ REL::ID(66913) };
#endif
		return func(bstarray, size, a6);
	}

	inline uint64_t AllocInstanceDataItem1(uintptr_t ptrToVariable, uint64_t max, uint32_t a2, char a3)
	{
		using func_t = decltype(&AllocInstanceDataItem1);
#if ANNIVERSARY_EDITION
		REL::Relocation<func_t> func{ REL::ID(66859) };

#else
		REL::Relocation<func_t> func{ REL::ID(66859) };
#endif
		return func(ptrToVariable, max, a2, a3);
	}

	inline uint64_t ClearInstanceDataItem(RE::BGSQuestInstanceText* questInstanceText)
	{
		using func_t = decltype(&ClearInstanceDataItem);
#if ANNIVERSARY_EDITION
		REL::Relocation<func_t> func{ REL::ID(23413) };

#else
		REL::Relocation<func_t> func{ REL::ID(23413) };
#endif
		return func(questInstanceText);
	}

	inline void ClearInstanceStringData(RE::BSTArray<RE::BGSQuestInstanceText::StringData>* instanceText, int size)
	{
		using func_t = decltype(&ClearInstanceStringData);
#if ANNIVERSARY_EDITION
		REL::Relocation<func_t> func{ REL::ID(23445) };

#else
		REL::Relocation<func_t> func{ REL::ID(23445) };
#endif
		return func(instanceText, size);
	}

	inline void ClearInstanceValueData(RE::BSTArray<RE::BGSQuestInstanceText::GlobalValueData>* instanceText, int size)
	{
		using func_t = decltype(&ClearInstanceValueData);
#if ANNIVERSARY_EDITION
		REL::Relocation<func_t> func{ REL::ID(23444) };

#else
		REL::Relocation<func_t> func{ REL::ID(23444) };
#endif
		return func(instanceText, size);
	}

	inline void SetArrayCount(uint64_t array, int newSize)
	{
		const auto arrayCountAddress = array + static_cast<uint64_t>(0x10);
		auto size = reinterpret_cast<int*>(arrayCountAddress);
		*size = newSize;
	}

	inline auto LoadInstanceData(BinaryReader& rf, RE::TESQuest* quest)
	{
		const auto instanceId = rf.ReadUint32();
		if (quest != nullptr) {
			quest->currentInstanceID = instanceId;
		}

		const uint32_t instanceDataTotalCount = rf.ReadUint32();
		if (instanceDataTotalCount == 0) {
			return;
		}

		if (quest == nullptr) {
			for (uint32_t i = 0; i < instanceDataTotalCount; ++i) {
				const auto id = rf.ReadUint32();

				const auto stringDataSize = rf.ReadUint32();
				for (uint32_t i = 0; i < stringDataSize; ++i) {
					const auto aliasId = rf.ReadUint32();
					const auto fullNameFormId = rf.ReadUint32();
				}

				const auto valueSize = rf.ReadUint32();
				for (int i = 0; i < valueSize; ++i) {
					const auto size = rf.ReadUint64();
					const auto editorId = rf.ReadFixedLengthString(size);
					const auto value = rf.ReadFloat();
				}
				const auto journalStage = rf.ReadUint16();
				const auto journalStageItem = rf.ReadInt8();
			}

			return;
		}

		//AllocInstanceArray(&quest->instanceData, instanceDataTotalCount, instanceDataTotalCount, 0, 0, 8);
		AllocInstanceArray2(reinterpret_cast<uint64_t>(&quest->instanceData), instanceDataTotalCount, 8);
		SetArrayCount(reinterpret_cast<uint64_t>(&quest->instanceData), instanceDataTotalCount);
		REL::Offset offset(static_cast<uint64_t>(0x1EBD280));

		for (uint32_t i = 0; i < instanceDataTotalCount; ++i) {
			const auto newItem = reinterpret_cast<RE::BGSQuestInstanceText*>(AllocInstanceDataItem1(offset.address(), 64, 0, 0));
			ClearInstanceDataItem(newItem);

			//auto newInstance = new RE::BGSQuestInstanceText();
			quest->instanceData[i] = newItem;
			const auto newInstance = quest->instanceData[i];

			newInstance->id = rf.ReadUint32();

			const uint32_t stringDataSize = rf.ReadUint32();
			ClearInstanceStringData(&newInstance->stringData, stringDataSize);
			//const auto newStringDataArray = new RE::BSTArray<RE::BGSQuestInstanceText::StringData>(stringDataSize);
			for (uint32_t x = 0; x < stringDataSize; ++x) {
				newInstance->stringData[x].aliasID = rf.ReadUint32();
				newInstance->stringData[x].fullNameFormID = rf.ReadUint32();
			}
			//newInstance->stringData = *newStringDataArray;

			if (std::string(quest->formEditorID) == std::string("MQ102")) {
				//newInstance->stringData = oldStringData;
				const auto asdfff = 1;
			}

			const uint32_t valueSize = rf.ReadUint32();
			ClearInstanceValueData(&newInstance->valueData, valueSize);
			for (uint32_t x = 0; x < valueSize; ++x) {
				const auto size = rf.ReadUint64();
				auto editorId = rf.ReadFixedLengthString(size);
				const auto value = rf.ReadFloat();
				newInstance->valueData[x].global = FindGlobalForm(editorId);
				newInstance->valueData[x].value = value;
			}

			newInstance->journalStage = rf.ReadUint16();
			newInstance->journalStageItem = rf.ReadInt8();

			//quest->instanceData.emplace_back(new RE::BGSQuestInstanceText());
		}
	}

	inline RE::TESQuest* FindQuestByEditorId(const std::string& editorId)
	{
		for (const auto quest : allQuests) {
			const auto questEditorId = std::string(quest->formEditorID);
			if (editorId == questEditorId) {
				return quest;
			}
		}

		return nullptr;
	}

	inline RE::BGSQuestObjective* FindQuestObjectiveByIndex(RE::TESQuest* quest, uint32_t index)
	{
		if (quest != nullptr) {
			for (const auto objective : quest->objectives) {
				if (objective != nullptr) {
					if (objective->index == index) {
						return objective;
					}
				}
			}
		}
		return nullptr;
	}

	inline auto SaveCharacterQuestObjectives(BinaryWriter& wf)
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto objectives = player->GetPlayerRuntimeData().objectives;

		wf.WriteUint64(objectives.size());

		for (const auto objective : objectives) {
			const auto formEditorId = std::string(objective.Objective->ownerQuest->formEditorID);
			const auto objectiveIndex = objective.Objective->index;

			wf.WriteUint64(formEditorId.size());
			wf.WriteFixedLengthString(formEditorId);
			wf.WriteUint16(objectiveIndex);
			wf.WriteUint32(objective.instanceID);
			wf.WriteUint8(static_cast<uint8_t>(objective.InstanceState));
		}
	}

	inline auto LoadCharacterQuestObjectives(BinaryReader& rf)
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		auto playerObjectives = player->GetPlayerRuntimeData().objectives;
		const auto newCount = rf.ReadUint64();

		for (uint64_t i = 0; i < playerObjectives.size(); ++i) {
			const auto address = reinterpret_cast<uint64_t*>(&playerObjectives[i]);
			const auto address2 = reinterpret_cast<uint64_t*>(reinterpret_cast<uint64_t>(&playerObjectives[i]) + static_cast<uint64_t>(0x8));
			*address = 0;
			*address2 = 0;
		}

		playerObjectives.resize(newCount);

		//AllocInstanceArray2(reinterpret_cast<uint64_t>(&player->objectives), newCount, 8);
		SetArrayCount(reinterpret_cast<uint64_t>(&playerObjectives), newCount);

		for (uint64_t i = 0; i < newCount; ++i) {
			const auto address = reinterpret_cast<uint64_t*>(&playerObjectives[i]);
			const auto address2 = reinterpret_cast<uint64_t*>(reinterpret_cast<uint64_t>(&playerObjectives[i]) + static_cast<uint64_t>(0x8));
			*address = 0;
			*address2 = 0;
		}

		for (uint64_t i = 0; i < newCount; ++i) {
			const auto editorIdSize = rf.ReadUint64();
			const auto editorId = rf.ReadFixedLengthString(editorIdSize);
			const auto quest = FindQuestByEditorId(editorId);
			if (quest == nullptr) {
				rf.ReadUint16();
				rf.ReadUint32();
				rf.ReadUint8();
				continue;
			}

			const auto questObjective = FindQuestObjectiveByIndex(quest, rf.ReadUint16());
			const auto instanceId = rf.ReadUint32();
			const auto instanceState = rf.ReadUint8();

			const auto arrayObjective = &(playerObjectives[static_cast<uint32_t>(i)]);

			if (questObjective != nullptr) {
				arrayObjective->Objective = questObjective;
				arrayObjective->instanceID = instanceId;
				arrayObjective->InstanceState = static_cast<RE::QUEST_OBJECTIVE_STATE>(instanceState);
			}
		}
	}

	inline auto SaveQuest(BinaryWriter& wf, RE::TESQuest* quest)
	{
		const auto address = reinterpret_cast<uintptr_t*>(quest);

		const auto editorId = std::string(quest->formEditorID);
		if (editorId.empty()) {
			wf.WriteUint64(0);
			return;
		}

		wf.WriteUint64(editorId.size());
		wf.WriteFixedLengthString(editorId);

		wf.WriteUint16(static_cast<uint16_t>(quest->data.flags.get()));

		SaveQuestStages(wf, quest);
		SaveObjectives(wf, quest);
		SaveInstanceData(wf, quest);

		wf.WriteUint8(quest->alreadyRun);		
	}

	

	

	inline auto LoadNextQuest(BinaryReader& rf)
	{
		const auto stringSize = rf.ReadUint64();
		if (stringSize == 0 || stringSize > 300) {
			return;
		}

		const auto editorId = std::string(rf.ReadFixedLengthString(stringSize));

		if (editorId == std::string("MQ102")) {
			int bob = 1;
		}

		const auto quest = FindQuestByEditorId(editorId);

		uint16_t questDataFlags = 0;
		questDataFlags = rf.ReadUint16();
		if (quest != nullptr) {
			quest->data.flags = static_cast<RE::QuestFlag>(questDataFlags);
		}
		

		LoadQuestStages(rf, quest);
		LoadObjectives(rf, quest);
		LoadInstanceData(rf, quest);

		const bool alreadyRun = rf.ReadUint8();
		if (quest != nullptr) {
			quest->alreadyRun = alreadyRun;

			const auto newQuestStage = FindHighestQuestStage(quest);
			quest->currentStage = newQuestStage;
		}

		const auto bob = 1;
	}

	inline auto QuestSavePath()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto playerName = std::string(player->GetName());

		PWSTR pathW;
		SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &pathW);
		std::wstring strpath(pathW);
		CoTaskMemFree(pathW);

		std::string path = utf8_encode(pathW);
		path += R"(\my games\Skyrim Special Edition\JCUser\Proteus\Proteus_Character_Quests_)";
		path += playerName;
		path += ".data";

		return path;
	}

	inline auto SetupQuestCache()
	{
		if (allQuests.empty()) {
			const auto quests = Item::ProteusGetAllInModByFormId(nullptr, 0, nullptr, RE::FormType::Quest, "Skyrim.esm");
			for (const auto form : quests) {
				const auto quest = reinterpret_cast<RE::TESQuest*>(form);
				if (quest != nullptr) {
					allQuests.push_back(quest);
					cachedQuests.insert({ { std::string(quest->formEditorID) }, { quest } });
				}
			}
		}
	}

	inline auto QuestsTestSave(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag) {
		SetupQuestCache();

		const auto path = QuestSavePath();

		BinaryWriter writer(path);

		writer.WriteUint64(allQuests.size());


		for (const auto questForm : allQuests) {
			const auto quest = static_cast<RE::TESQuest*>(questForm);
			SaveQuest(writer, quest);
		}

		SaveCharacterQuestObjectives(writer);

		writer.Flush();
		logger::info("Saved {} quests", allQuests.size());
		return allQuests.size();
	}

	inline auto QuestsTestLoad(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag)
	{
		if (allQuests.empty()) {
			const auto quests = Item::ProteusGetAllByFormId(tag, RE::FormType::Quest);
			for (auto form : quests) {
				const auto quest = static_cast<RE::TESQuest*>(form);
				if (quest != nullptr) {
					allQuests.push_back(quest);
				}
			}
		}

		const auto path = QuestSavePath();

		BinaryReader reader(path);

		const auto numQuestsToLoad = reader.ReadUint64();
		for (uint64_t i = 0; i < numQuestsToLoad; ++i) {
			if (i >= 1609) {
				const auto bob = 1;
			}
			LoadNextQuest(reader);
		}

		LoadCharacterQuestObjectives(reader);
		

		logger::info("Loaded {} quests", numQuestsToLoad);
		return numQuestsToLoad;
	}
	
	inline void Bind(VM& a_vm)
	{
		BIND(QuestsTestSave);
		BIND(QuestsTestLoad);
		
		logger::info("Registered Proteus Quest functions");
	}
}
