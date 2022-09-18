#pragma once

#include <string>
#include <type_traits>

class SettingBase
{
public:
	SettingBase(const std::string& name);

	virtual void Load(const char* iniFileName, const char* category) = 0;
	virtual void Save(const char* iniFileName, const char* category) = 0;

protected:
	std::string name_;
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

	inline void Load(const char* iniFileName, const char* category) override
	{
		LoadInternal(iniFileName, category);
	}

	inline void Save(const char* iniFileName, const char* category) override
	{
		SaveInternal(iniFileName, category);
	}

private:
	inline void LoadInternal(const char* iniFileName, const char* category);
	inline void SaveInternal(const char* iniFileName, const char* category);

private:
	const T defaultVal_;
	T val_;
};

template<typename T>
inline void Setting<T>::LoadInternal(const char* iniFileName, const char* category)
{
}

template<typename T>
inline void Setting<T>::SaveInternal(const char* iniFileName, const char* category)
{
}

template<>
inline void Setting<bool>::LoadInternal(const char* iniFileName, const char* category)
{
	val_ = GetPrivateProfileIntA(category, name_.c_str(), defaultVal_ ? 1 : 0, iniFileName) == 1;
}

template<>
inline void Setting<bool>::SaveInternal(const char* iniFileName, const char* category)
{
	if (WritePrivateProfileStringA(category, name_.c_str(), std::to_string(val_ ? 1 : 0).c_str(), iniFileName) == FALSE)
	{
		OutputDebugStringA("Failed to write setting to file\n");
		OutputDebugStringA(std::to_string(GetLastError()).c_str());
		OutputDebugStringA("\n");
	}
}

template<>
inline void Setting<int>::LoadInternal(const char* iniFileName, const char* category)
{
	val_ = GetPrivateProfileIntA(category, name_.c_str(), defaultVal_, iniFileName);
}

template<>
inline void Setting<int>::SaveInternal(const char* iniFileName, const char* category)
{
	WritePrivateProfileStringA(category, name_.c_str(), std::to_string(val_).c_str(), iniFileName);
}

template<>
inline void Setting<std::string>::LoadInternal(const char* iniFileName, const char* category)
{
	val_.resize(255);
	const DWORD size = GetPrivateProfileStringA(category, name_.c_str(), defaultVal_.c_str(), val_.data(), val_.size(), iniFileName);
	val_.resize(size);
}

template<>
inline void Setting<std::string>::SaveInternal(const char* iniFileName, const char* category)
{
	WritePrivateProfileStringA(category, name_.c_str(), val_.c_str(), iniFileName);
}

namespace Settings
{
void LoadSettings();
void SaveSettings();
}