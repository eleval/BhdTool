#pragma once

#include <cstdint>
#include <vector>

void PatchCode(size_t address, const uint8_t* code, size_t codeSize);
void PatchCode(size_t address, const std::vector<uint8_t> code); void PatchCodeWithNops(size_t address, size_t count);