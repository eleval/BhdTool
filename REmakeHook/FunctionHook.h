#pragma once

#include "CodePatch.h"

class FunctionHook
{
public:
	FunctionHook();

	void Set(size_t address, void* funcPtr, size_t len = 5);

	void Apply();
	void Remove();

private:
	CodePatch codePatch_;
};

class TrampHook
{
public:
	TrampHook();

	void Set(char* src, char* dst, size_t len);

	void Apply();
	void Remove();

	void* GetGateway();

private:
	CodePatch codePatch_;
	char* gateway_;
};