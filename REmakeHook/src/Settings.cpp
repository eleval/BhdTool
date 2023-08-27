#include "pch.h"

#include "Settings.h"

#include <vector>

namespace
{
	constexpr const wchar_t* SettingsFileName = L"bhdtool.ini";
	constexpr const wchar_t* DefaultSettingsCategory = L"Settings";

	std::wstring settingsFilePath_;

	// Got to do this because of static initialization order
	std::vector<SettingBase*>& GetSettings()
	{
		static std::vector<SettingBase*> settings_;
		return settings_;
	}

	std::vector<SettingBase*>& settings_ = GetSettings();
}

SettingBase::SettingBase(const std::string& name) :
	name_(name.begin(), name.end())
{
	GetSettings().push_back(this);
}

void Settings::LoadSettings()
{
	wchar_t buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::wstring modulePath = buffer;
	const auto pos = modulePath.find_last_of(L"\\/");
	if (pos != std::wstring::npos)
	{
		modulePath = modulePath.substr(0, pos);
	}
	settingsFilePath_ = modulePath + L"\\" + std::wstring(SettingsFileName);

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