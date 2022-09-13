#include "pch.h"

#include "CodePatch.h"

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

std::vector<uint8_t> GetCode(size_t address, size_t size)
{
	std::vector<uint8_t> code;
	code.resize(size);
	SIZE_T bytesRead = 0;
	ReadProcessMemory(GetCurrentProcess(), (void*)address, code.data(), size, &bytesRead);

	return code;
}

void CodePatch::AddCode(size_t address, std::vector<uint8_t> code)
{
	Code codePatch;
	codePatch.address = address;
	codePatch.newCode = std::move(code);
	codePatch.oldCode = GetCode(address, code.size());
	codes_.push_back(codePatch);
}

void CodePatch::AddNops(size_t address, size_t count)
{
	Code codePatch;
	codePatch.address = address;
	codePatch.newCode.resize(count, 0x90);
	codePatch.oldCode = GetCode(address, count);
	codes_.push_back(codePatch);
}

void CodePatch::ApplyPatch()
{
	for (const Code& code : codes_)
	{
		PatchCode(code.address, code.newCode);
	}
}

void CodePatch::RemovePatch()
{
	for (const Code& code : codes_)
	{
		PatchCode(code.address, code.oldCode);
	}
}