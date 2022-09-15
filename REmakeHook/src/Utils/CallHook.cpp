#include "pch.h"

#include "CallHook.h"

#include <cassert>

CallHook::CallHook()
{
}

void CallHook::Set(size_t address, void* funcPtr, size_t len)
{
	size_t funcAddress = reinterpret_cast<size_t>(funcPtr);
	funcAddress = funcAddress - address - 5;
	uint8_t* funcAddressBytes = reinterpret_cast<uint8_t*>(&funcAddress);
	codePatch_.AddNops(address, len);
	codePatch_.AddCode(address, { 0xE8, funcAddressBytes[0], funcAddressBytes[1], funcAddressBytes[2], funcAddressBytes[3] });
}

void CallHook::Apply()
{
	codePatch_.Apply();
}

void CallHook::Remove()
{
	codePatch_.Remove();
}