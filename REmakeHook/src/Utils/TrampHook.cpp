#include "pch.h"

#include "Utils/TrampHook.h"

#include <cassert>

TrampHook::TrampHook()
{
}

void TrampHook::Set(char* src, char* dst, size_t len)
{
	assert(len >= 5);

	gateway_ = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(gateway_, src, len);
	uintptr_t jumpAddy = (uintptr_t)(src - gateway_ - 5);
	*(gateway_ + len) = (char)0xE9;
	*(uintptr_t*)(gateway_ + len + 1) = jumpAddy;

	codePatch_.AddNops((size_t)src, len);
	uint8_t hookRelAdd[4];
	*(uintptr_t*)(hookRelAdd) = (uintptr_t)(dst - src - 5);
	codePatch_.AddCode((size_t)src, { 0xE9, hookRelAdd[0], hookRelAdd[1], hookRelAdd[2], hookRelAdd[3] });
}

void TrampHook::Apply()
{
	codePatch_.Apply();
}

void TrampHook::Remove()
{
	codePatch_.Remove();
}

void* TrampHook::GetGateway()
{
	return gateway_;
}