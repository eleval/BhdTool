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

	void Set(char* address, char* funcPtr, size_t len);

	void Apply();
	void Remove();

	void* GetGateway();

private:
	FunctionHook hook_;
	char* gateway_;
};