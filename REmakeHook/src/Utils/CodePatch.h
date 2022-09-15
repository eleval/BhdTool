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

	template<typename T>
	void AddCode_Internal(std::vector<uint8_t>& code, T arg)
	{
		static_assert(std::is_fundamental_v<T>);
		const uint8_t* argPtr = (uint8_t*)(&arg);
		for (size_t i = 0; i < sizeof(T); ++i)
		{
			code.push_back(argPtr[i]);
		}
	}

	template<typename T, typename...Args>
	void AddCode_Internal(std::vector<uint8_t>& code, T arg, Args...args)
	{
		AddCode_Internal(code, arg);
		AddCode_Internal(code, args...);
	}

public:
	CodePatch() = default;

	void AddCode(size_t address, std::vector<uint8_t> code);
	void AddNops(size_t address, size_t count);

	template<typename...Args>
	void AddCode(size_t address, Args...args)
	{
		std::vector<uint8_t> code;
		code.reserve(sizeof...(Args));
		AddCode_Internal(code, args...);
		
		AddCode(address, code);
	}

	void Apply();
	void Remove();

private:
	std::vector<Code> codes_;
};