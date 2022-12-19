#include "pch.h"

#include "Utils/Memory.h"

void Mem::WriteMemory(size_t address, const void* bytes, size_t size)
{
	SIZE_T bytesWritten = 0;
	WriteProcessMemory(GetCurrentProcess(), (void*)address, bytes, size, &bytesWritten);
}

void Mem::WriteMemory(size_t address, const std::vector<uint8_t>& bytes)
{
	WriteMemory(address, bytes.data(), bytes.size());
}

void Mem::ReadMemory(size_t address, void* bytes, size_t size)
{
	SIZE_T bytesRead = 0;
	ReadProcessMemory(GetCurrentProcess(), (void*)address, bytes, size, &bytesRead);
}

std::vector<uint8_t> Mem::ReadMemory(size_t address, size_t size)
{
	std::vector<uint8_t> data(size);
	ReadMemory(address, data.data(), size);
	return data;
}