#pragma once

#define BHD_TOOL_VERSION_MAJOR 0
#define BHD_TOOL_VERSION_MINOR 3

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_VERSION(major, minor) STR(major) "." STR(minor)

constexpr const char* BuildVersion = BUILD_VERSION(BHD_TOOL_VERSION_MAJOR, BHD_TOOL_VERSION_MINOR);

namespace BhdTool
{
	void Init();
	void Update();
	void Toggle();
}