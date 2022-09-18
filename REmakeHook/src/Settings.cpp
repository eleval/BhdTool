#include "pch.h"

#include "Settings.h"

#include <vector>

namespace
{
	constexpr const char* SettingsFileName = "bhdtool.ini";
	constexpr const char* DefaultSettingsCategory = "Settings";

	std::string settingsFilePath_;

	// Got to do this because of static initialization order
	std::vector<SettingBase*>& GetSettings()
	{
		static std::vector<SettingBase*> settings_;
		return settings_;
	}

	std::vector<SettingBase*>& settings_ = GetSettings();
}

SettingBase::SettingBase(const std::string& name) :
	name_(name)
{
	GetSettings().push_back(this);
}

void Settings::LoadSettings()
{
	char buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string modulePath = buffer;
	const auto pos = modulePath.find_last_of("\\/");
	if (pos != std::string::npos)
	{
		modulePath = modulePath.substr(0, pos);
	}
	settingsFilePath_ = modulePath + "\\" + std::string(SettingsFileName);

	for (SettingBase* setting : settings_)
	{
		setting->Load(settingsFilePath_.c_str(), DefaultSettingsCategory);
	}
}

void Settings::SaveSettings()
{
	for (SettingBase* setting : settings_)
	{
		setting->Save(settingsFilePath_.c_str(), DefaultSettingsCategory);
	}
}