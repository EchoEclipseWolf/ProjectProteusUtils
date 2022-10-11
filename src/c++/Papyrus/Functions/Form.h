#pragma once
#include "Papyrus/Game/EditorIdCache.h"
#include "Papyrus/Game/KeywordManager.h"

namespace Papyrus::Form
{
	//Credit https://github.com/powerof3/PapyrusExtenderSSE Papyrus Extender SSE
	namespace fave_util
	{
		namespace item
		{
			inline void favorite(RE::InventoryChanges* a_changes, RE::InventoryEntryData* a_entryData, RE::ExtraDataList* a_list)
			{
				using func_t = decltype(&favorite);
				auto func = (func_t)RELOCATION_ID(15858, 16098).address();
				return func(a_changes, a_entryData, a_list);
			}

			inline void unfavorite(RE::InventoryChanges* a_changes, RE::InventoryEntryData* a_entryData, RE::ExtraDataList* a_list)
			{
				using func_t = decltype(&unfavorite);
				auto func = (func_t)RELOCATION_ID(15859, 16099).address();
				return func(a_changes, a_entryData, a_list);
			}

			inline RE::ExtraDataList* get_hotkeyed(RE::InventoryEntryData* a_changes)
			{
				if (a_changes->extraLists) {
					for (const auto& xList : *a_changes->extraLists) {
						const auto hotkey = xList->HasType<RE::ExtraHotkey>();
						if (hotkey) {
							return xList;
						}
					}
				}
				return nullptr;
			}
		}

		namespace magic
		{
			inline void favorite(RE::MagicFavorites* a_magicFavorites, RE::TESForm* a_form)
			{
				using func_t = decltype(&favorite);
				auto func = (func_t)RELOCATION_ID(51121, 52004).address();
				return func(a_magicFavorites, a_form);
			}

			inline void unfavorite(RE::MagicFavorites* a_magicFavorites, RE::TESForm* a_form)
			{
				using func_t = decltype(&unfavorite);
				auto func = (func_t)RELOCATION_ID(51122, 52005).address();
				return func(a_magicFavorites, a_form);
			}
		}
	}

	inline auto ProteusMarkItemAsFavorite(VM*, StackID, RE::StaticFunctionTag*, RE::TESForm* a_form)
	{
		using namespace fave_util;

		if (!a_form) {
			return 0;
		}

		if (a_form->Is(RE::FormType::Spell, RE::FormType::Shout)) {
			const auto magicFavorites = RE::MagicFavorites::GetSingleton();
			if (magicFavorites && std::ranges::find(magicFavorites->spells, a_form) == magicFavorites->spells.end()) {
				magic::favorite(magicFavorites, a_form);
			}
		} else {
			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto xContainer = player ? player->extraList.GetByType<RE::ExtraContainerChanges>() : nullptr;

			if (const auto invChanges = xContainer ? xContainer->changes : nullptr) {
				for (auto inv = player->GetInventory(); const auto& [item, data] : inv) {
					const auto& [count, entry] = data;
					if (count > 0 && item == a_form && !item::get_hotkeyed(entry.get())) {
						const auto extralist = entry->extraLists ? entry->extraLists->front() : nullptr;
						item::favorite(invChanges, entry.get(), extralist);
						break;
					}
				}
			}
		}
		return 1;
	}

	inline auto ProteusUnmarkItemAsFavorite(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* a_form)
	{
		using namespace fave_util;
		if (!a_form) {
			return 0;
		}

		if (a_form->Is(RE::FormType::Spell, RE::FormType::Shout)) {
			const auto magicFavorites = RE::MagicFavorites::GetSingleton();
			if (magicFavorites && std::ranges::find(magicFavorites->spells, a_form) != magicFavorites->spells.end()) {
				magic::unfavorite(magicFavorites, a_form);
			}
		} else {
			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto xContainer = player ? player->extraList.GetByType<RE::ExtraContainerChanges>() : nullptr;

			if (const auto invChanges = xContainer ? xContainer->changes : nullptr) {
				for (auto inv = player->GetInventory(); const auto& [item, data] : inv) {
					const auto& [count, entry] = data;
					if (count > 0 && item == a_form) {
						if (const auto extralist = item::get_hotkeyed(entry.get())) {
							item::unfavorite(invChanges, entry.get(), extralist);
							logger::info("Marked Item as Unfavorite: {}", item->GetName());
						}
						break;
					}
				}
			}
		}

		return 1;
	}

	inline RE::BSFixedString ProteusGetFormEditorID(VM*, StackID, RE::StaticFunctionTag*, RE::TESForm* a_form)
	{
		if (!a_form) {
			return RE::BSFixedString();
		}

		EditorIdCache::EditorID::GetSingleton()->FillMap();

		const auto editorId = EditorIdCache::EditorID::GetSingleton()->GetEditorID(a_form->GetFormID());

		return editorId;
	}

	inline auto ProteusRemoveKeywordOnForm(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::TESForm* a_form,
		RE::BGSKeyword* a_keyword)
	{
		using namespace Form;

		if (!a_form) {
			logger::info("RemoveKeywordOnForm: Form is None");
			return false;
		}
		if (!a_keyword) {
			logger::info("RemoveKeywordOnForm: Keyword is None");
			return false;
		}

		return Keyword::KeywordManager::GetSingleton()->Remove(a_form, a_keyword);
	}

	inline auto ProteusAddKeywordToForm(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::TESForm* a_form,
		RE::BGSKeyword* a_keyword)
	{
		using namespace Form;

		if (!a_form) {
			logger::info("AddKeywordToForm: Form is None");
			return 0;
		}
		if (!a_keyword) {
			logger::info("AddKeywordToForm: Keyword is None");
			return 0;
		}

		Keyword::KeywordManager::GetSingleton()->Add(a_form, a_keyword);
		logger::info("AddKeywordToForm: Success");
		return 1;
	}

	inline auto ProteusReplaceKeywordOnForm(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*,
		RE::TESForm* a_form,
		const RE::BGSKeyword* a_remove,
		RE::BGSKeyword* a_add)
	{
		if (!a_form) {
			logger::info("ReplaceKeywordOnForm: Form is None");
			return 0;
		}
		if (!a_remove) {
			logger::info("ReplaceKeywordOnForm: Remove Keyword is None");
			return 0;
		}
		if (!a_add) {
			logger::info("ReplaceKeywordOnForm: Add Keyword is None");
			return 0;
		}

		if (const auto keywordForm = a_form->As<RE::BGSKeywordForm>(); keywordForm) {
			if (keywordForm->keywords) {
				bool found = false;
				std::uint32_t removeIndex = 0;
				for (std::uint32_t i = 0; i < keywordForm->numKeywords; i++) {
					if (const auto keyword = keywordForm->keywords[i]) {
						if (keyword == a_add) {
							return 0;
						}
						if (keyword == a_remove) {
							removeIndex = i;
							found = true;
							break;
						}
					}
				}
				if (found) {
					keywordForm->keywords[removeIndex] = a_add;
				}
			}
		}
		logger::info("ReplaceKeywordOnForm: Success");
		return 1;
	}

	inline bool ProteusIsFormInMod(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::TESForm* a_form, RE::BSFixedString a_modName)
	{
		if (!a_form) {
			logger::info("IsFormInMod: Form is empty or null");
			return false;
		}

		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		const auto modInfo = dataHandler ? dataHandler->LookupModByName(a_modName) : nullptr;

		return modInfo ? modInfo->IsFormInMod(a_form->GetFormID()) : false;
	}

	inline void Bind(VM& a_vm)
	{
		BIND(ProteusMarkItemAsFavorite);
		BIND(ProteusUnmarkItemAsFavorite);
		BIND(ProteusGetFormEditorID);
		BIND(ProteusAddKeywordToForm);
		BIND(ProteusRemoveKeywordOnForm);
		BIND(ProteusReplaceKeywordOnForm);
		BIND(ProteusIsFormInMod);
		
		logger::info("Registered Proteus Form functions");
	}
}

/* MIT License

Copyright(c) 2021 powerofthree

Permission is hereby granted,
free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies
or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",
WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER
LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
