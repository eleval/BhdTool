#pragma once

#include <cstdint>
#include <vector>

void PatchCode(size_t address, const uint8_t* code, size_t codeSize);
void PatchCode(size_t address, const std::vector<uint8_t> code);
void PatchCodeWithNops(size_t address, size_t count);
std::vector<uint8_t> GetCode(size_t address, size_t size);

class CodePatch
{
	struct Code
	{
		size_t address;
		std::vector<uint8_t> oldCode;
		std::vector<uint8_t> newCode;
	};

public:
	CodePatch() = default;

	void AddCode(size_t address, std::vector<uint8_t> code);
	void AddNops(size_t address, size_t count);

	void ApplyPatch();
	void RemovePatch();

private:
	std::vector<Code> codes_;
};