#pragma once

#include "Utils/CodePatch.h"

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