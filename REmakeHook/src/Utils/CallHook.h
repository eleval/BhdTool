#pragma once

#include "CodePatch.h"

class CallHook
{
public:
	CallHook();

	void Set(size_t address, void* funcPtr, size_t len = 5);

	void Apply();
	void Remove();

private:
	CodePatch codePatch_;
};