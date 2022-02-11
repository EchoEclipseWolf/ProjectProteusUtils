#pragma once
#include "DataMap.h"
#include "robin_hood.h"
#include "RE/Skyrim.h"

namespace Keyword
{
	enum : std::uint32_t
	{
		kRemove = 0,
		kAdd = 1
	};

	class KeywordManager final : public FormMapPair<RE::TESForm, RE::BGSKeyword>
	{
	public:
		[[nodiscard]] static KeywordManager* GetSingleton()
		{
			static KeywordManager singleton;
			return &singleton;
		}

	private:
		bool Process(RE::TESForm* a_form, RE::BGSKeyword* a_data, std::uint32_t a_index) override
		{
			if (const auto keywordForm = a_form->As<RE::BGSKeywordForm>(); keywordForm) {
				return a_index == kAdd ?
                           keywordForm->AddKeyword(a_data) :
                           keywordForm->RemoveKeyword(a_data);
			}
			return false;
		}

	protected:
		KeywordManager() = default;
		KeywordManager(const KeywordManager&) = delete;
		KeywordManager(KeywordManager&&) = delete;
		~KeywordManager() = default;

		KeywordManager& operator=(const KeywordManager&) = delete;
		KeywordManager& operator=(KeywordManager&&) = delete;
	};
}
