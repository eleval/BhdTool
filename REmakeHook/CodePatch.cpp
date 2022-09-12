#include "pch.h"

void PatchCode(size_t address, const uint8_t* code, size_t codeSize)
{
	SIZE_T bytesWritten = 0;
	WriteProcessMemory(GetCurrentProcess(), (void*)address, code, codeSize, &bytesWritten);
}

void PatchCode(size_t address, const std::vector<uint8_t> code)
{
	PatchCode(address, code.data(), code.size());
}

void PatchCodeWithNops(size_t address, size_t count)
{
	constexpr const uint8_t BigNop[] =
	{
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	};

	if (count > sizeof(BigNop))
	{
		std::vector<uint8_t> code;
		code.resize(count, 0x90);
		PatchCode(address, code.data(), count);
	}
	else
	{
		PatchCode(address, BigNop, count);
	}
}