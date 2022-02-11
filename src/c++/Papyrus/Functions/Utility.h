#pragma once

namespace Papyrus::Utility
{
	inline auto IsDLLLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag* tag)
	{
		logger::info("DLL Is Loaded and connected");
		return 1;
	}


	
	inline void Bind(VM& a_vm)
	{
		BIND(IsDLLLoaded);
		
		logger::info("Registered Proteus Utility functions");
	}
}
