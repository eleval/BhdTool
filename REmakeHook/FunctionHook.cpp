#include "pch.h"

#include "FunctionHook.h"

#include <cassert>

FunctionHook::FunctionHook()
{
}

void FunctionHook::Set(size_t address, void* funcPtr, size_t len)
{
	size_t funcAddress = reinterpret_cast<size_t>(funcPtr);
	funcAddress = funcAddress - address - 5;
	uint8_t* funcAddressBytes = reinterpret_cast<uint8_t*>(&funcAddress);
	codePatch_.AddNops(address, len);
	codePatch_.AddCode(address, { 0xE8, funcAddressBytes[0], funcAddressBytes[1], funcAddressBytes[2], funcAddressBytes[3] });
}

void FunctionHook::Apply()
{
	codePatch_.Apply();
}

void FunctionHook::Remove()
{
	codePatch_.Remove();
}

TrampHook::TrampHook()
{
}

void Hook(char* src, char* dst, int len)
{
	DWORD oProc;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oProc);
	memset(src, 0x90, len);
	uintptr_t relAddy = (uintptr_t)(dst - src - 5);
	*src = (char)0xE9;
	*(uintptr_t*)(src + 1) = (uintptr_t)relAddy;
	VirtualProtect(src, len, oProc, &oProc);
}

void TrampHook::Set(char* src, char* dst, size_t len)
{
	assert(len >= 5);
	//hook_.Set((size_t)src, dst, len);

	/*gateway_ = (uint8_t*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	std::memcpy(gateway_, address, len);
	const size_t jumpAddy = ((uintptr_t)address - (uintptr_t)gateway_) - 5;
	*(uint8_t*)((uintptr_t)gateway_ + len) = 0xE9;
	*(uintptr_t*)(gateway_ + len + 1) = jumpAddy;*/


	gateway_ = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(gateway_, src, len);
	uintptr_t jumpAddy = (uintptr_t)(src - gateway_ - 5);
	*(gateway_ + len) = (char)0xE9;
	*(uintptr_t*)(gateway_ + len + 1) = jumpAddy;
	Hook(src, dst, len);
}

void TrampHook::Apply()
{
	hook_.Apply();
}

void TrampHook::Remove()
{
	hook_.Remove();
}

void* TrampHook::GetGateway()
{
	return gateway_;
}