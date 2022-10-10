#include "Events.h"
#include "Papyrus/Papyrus.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "Papyrus/Game/EditorIdCache.h"

#if ANNIVERSARY_EDITIO

#else
#include "Papyrus/Game/versiondb.h"
#endif

#define NDEBUG

namespace
{
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
			Events::Register();
			break;
		}
	}

	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path) {
			stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("Proteus.log");
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		constexpr auto level = spdlog::level::trace;
#else
		constexpr auto level = spdlog::level::trace;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
	}
}


extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(REL::Version(1, 0, 0, 1));
	v.PluginName("Proteus");

	v.UsesAddressLibrary(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Proteus";
	a_info->version = 1;

	if (a_skse->IsEditor()) {
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		return false;
	}

	/* VersionDb db;
	if (!db.Load(1, 5, 97, 0)) {
		return false;
	}
	db.Dump("offsets-1.5.97.0.txt");*/

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("Proteus v{}"sv, "1.0.2");

	SKSE::Init(a_skse);

	const auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	const auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus->Register(Papyrus::Bind)) {
		logger::critical("Failed to register papyrus callback"sv);
		return false;
	}

	return true;
}
