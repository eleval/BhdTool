#pragma once

#include <cstdint>
#include <vector>

namespace Mem
{
	void WriteMemory(size_t address, const void* bytes, size_t size);
	void WriteMemory(size_t address, const std::vector<uint8_t>& bytes);

	void ReadMemory(size_t address, void* bytes, size_t size);
	std::vector<uint8_t> ReadMemory(size_t address, size_t size);

	template<typename T>
	void WriteMemory(size_t address, const T& data)
	{
		WriteMemory(address, &data, sizeof(T));
	}

	template<typename T>
	T ReadMemory(size_t address)
	{
		T data;
		ReadMemory(address, &data, sizeof(T));
		return data;
	}

	template<typename T>
	void ReadMemory(size_t address, T& outData)
	{
		ReadMemory(address, &outData, sizeof(T));
	}
}