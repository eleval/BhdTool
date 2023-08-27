#pragma once

#include <string>
#include <type_traits>

class SettingBase
{
public:
	SettingBase(const std::string& name);

	virtual void Load(const wchar_t* iniFileName, const wchar_t* category) = 0;
	virtual void Save(const wchar_t* iniFileName, const wchar_t* category) = 0;

protected:
	std::wstring name_;
};

template<typename T>
class Setting final : public SettingBase
{
	static_assert(std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, std::string>,
		"The only allowed Setting types are bool, int & std::string");
public:
	inline Setting(const std::string& name, T defaultVal) :
		SettingBase(name),
		defaultVal_(defaultVal),
		val_(defaultVal)
	{
	}

	inline const T& Get()
	{
		return val_;
	}

	inline void Set(const T& val)
	{
		val_ = val;
	}

	inline void Reset()
	{
		val_ = defaultVal_;
	}

	inline void Load(const wchar_t* iniFileName, const wchar_t* category) override
	{
		LoadInternal(iniFileName, category);
	}

	inline void Save(const wchar_t* iniFileName, const wchar_t* category) override
	{
		SaveInternal(iniFileName, category);
	}

private:
	inline void LoadInternal(const wchar_t* iniFileName, const wchar_t* category);
	inline void SaveInternal(const wchar_t* iniFileName, const wchar_t* category);

private:
	const T defaultVal_;
	T val_;
};

template<typename T>
inline void Setting<T>::LoadInternal(const wchar_t* iniFileName, const wchar_t* category)
{
}

template<typename T>
inline void Setting<T>::SaveInternal(const wchar_t* iniFileName, const wchar_t* category)
{
}

template<>
inline void Setting<bool>::LoadInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	val_ = GetPrivateProfileIntW(category, name_.c_str(), defaultVal_ ? 1 : 0, iniFileName) == 1;
}

template<>
inline void Setting<bool>::SaveInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	WritePrivateProfileStringW(category, name_.c_str(), std::to_wstring(val_ ? 1 : 0).c_str(), iniFileName);
}

template<>
inline void Setting<int>::LoadInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	val_ = GetPrivateProfileIntW(category, name_.c_str(), defaultVal_, iniFileName);
}

template<>
inline void Setting<int>::SaveInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	WritePrivateProfileStringW(category, name_.c_str(), std::to_wstring(val_).c_str(), iniFileName);
}

template<>
inline void Setting<std::string>::LoadInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	std::wstring wDefaultVal;
	wDefaultVal.assign(defaultVal_.begin(), defaultVal_.end());
	std::wstring wValue;
	wValue.resize(255);
	const DWORD size = GetPrivateProfileStringW(category, name_.c_str(), wDefaultVal.c_str(), wValue.data(), wValue.size(), iniFileName);
	wValue.resize(size);
	val_.resize(size);
	WideCharToMultiByte(CP_UTF8, 0, wValue.c_str(), -1, val_.data(), size, nullptr, nullptr);
}

template<>
inline void Setting<std::string>::SaveInternal(const wchar_t* iniFileName, const wchar_t* category)
{
	std::wstring wValue;
	wValue.assign(val_.begin(), val_.end());
	WritePrivateProfileStringW(category, name_.c_str(), wValue.c_str(), iniFileName);
}

namespace Settings
{
	void LoadSettings();
	void SaveSettings();
}