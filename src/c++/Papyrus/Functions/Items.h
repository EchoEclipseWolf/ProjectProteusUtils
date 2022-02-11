#pragma once
#include "Form.h"

namespace Papyrus::Item
{
	static inline std::map<RE::FormType, RE::BSTArray<RE::TESForm*>> _formCache;

	inline bool can_be_taken(const std::unique_ptr<RE::InventoryEntryData>& a_entry, bool a_noEquipped, bool a_noFavourited, bool a_noQuestItem)
	{
		//Credit https://github.com/powerof3/PapyrusExtenderSSE Papyrus Extender SSE
		if (a_noEquipped && a_entry->IsWorn()) {
			return false;
		}
		if (a_noFavourited && a_entry->IsFavorited()) {
			return false;
		}
		if (a_noQuestItem && a_entry->IsQuestObject()) {
			return false;
		}
		return true;
	}

	inline std::vector<RE::TESForm*> ProteusAddAllItemsToArray(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::TESObjectREFR* a_ref,
		bool a_noEquipped,
		bool a_noFavourited,
		bool a_noQuestItem)
	{
		std::vector<RE::TESForm*> result;

		if (!a_ref) {
			return result;
		}

		auto inv = a_ref->GetInventory();
		for (const auto& [item, data] : inv) {
			if (item->Is(RE::FormType::LeveledItem)) {
				continue;
			}
			const auto& [count, entry] = data;
			if (count > 0 && can_be_taken(entry, a_noEquipped, a_noFavourited, a_noQuestItem)) {
				result.push_back(item);
			}
		}

		return result;
	}

	inline auto GetAllFavoritedItems(RE::StaticFunctionTag* tag)
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

	inline auto GetCombinedIndex(const RE::TESFile* a_file)
	{
		return static_cast<uint32_t>(a_file->compileIndex + a_file->smallFileCompileIndex);
	}

	void AddForms(RE::FormType a_type)
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

	inline auto ProteusGetItemCount(RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
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

	inline auto ProteusGetItemBySearch(RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
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

	inline auto ProteusGetItemEditorIdBySearch(VM* vm, StackID sid, RE::StaticFunctionTag* tag, RE::BSFixedString containsName, RE::FormType formType)
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
					const auto name = Papyrus::Form::ProteusGetFormEditorID(vm, sid, tag, item);
					if (name != nullptr) {
						const auto strName = std::string(name);

						if (auto it = std::ranges::search(strName, strContains
							,
							[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }).begin(); it != strName.end()) {
							result.emplace_back(item);
							count++;
						}
					}
				}
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			logger::info("Found {} editor items in the game", count);
			logger::info("		Total Found Time: {}", diff.count());
		}

		return result;
	}

	inline auto ProteusGetAllByFormId(RE::StaticFunctionTag* tag, RE::FormType formType)
	{
		const auto start = std::chrono::steady_clock::now();

		std::vector<RE::TESForm*> result;

		AddForms(formType);

		if (_formCache.contains(formType)) {
			const auto items = _formCache[formType];
			int count = items.size();

			for (auto item : items) {
				result.push_back(item);
			}

			const auto end = std::chrono::steady_clock::now();
			const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

			logger::info("Found {} total form items", count);
			logger::info("		Total Found Time: {}", diff.count());
		}

		return result;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(GetAllFavoritedItems);
		BIND(ProteusGetItemCount);
		BIND(ProteusGetItemBySearch);
		BIND(ProteusGetItemEditorIdBySearch);
		BIND(ProteusGetAllByFormId);
		BIND(ProteusAddAllItemsToArray);

		logger::info("Registered Proteus Item functions");
	}
}
