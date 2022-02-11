#pragma once

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

#include <memory>
#include <string>

#include <frozen/map.h>
#include <ranges>
#include <robin_hood.h>
#include <xbyak/xbyak.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

namespace logger = SKSE::log;
namespace stl = SKSE::stl;

#define DLLEXPORT __declspec(dllexport)

#include "Common.h"
