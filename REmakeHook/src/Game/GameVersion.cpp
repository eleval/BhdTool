#include "pch.h"

#include "GameVersion.h"

#include "picosha2/picosha2.h"

#include <array>
#include <string>
#include <vector>

namespace
{
	constexpr const std::array GameVersionHashes =
	{
		"0ab61bada34783a684490858e2005bbd2e9a1b1353eaaad44337bf7abb773b72",	// V_3831846201811674141
		"d18cdf473c01afbd90ce1df0c3f3bb4a3d6c4ebfe4d792a035894c63a2790f8f",	// V_7029559557684186662
		"0cb0dc1d2f8d9b2e56de380c39dab3ed93fba10fc063a85e0c5b0623f1869345",	// V_7029559557684186662_Steamless
		"1f4b8b17a34e65a518508e00002fdb02c5c72fadbb8bc778045d66bbfb8ce28a", // V_9087090834863155228
		"6f060443b0ef58bbf79bbe5f3b13ea9664f3fbd5ead3728d65bcac32f6a82baf", // V_9087090834863155228_Steamless
	};
	static_assert(GameVersionHashes.size() == static_cast<int>(GameVersion::Unknown));
}

GameVersion GetGameVersion()
{
	// Check game's executable SHA256
	wchar_t buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(nullptr, buffer, MAX_PATH);

	FILE* file = nullptr;
	if (_wfopen_s(&file, buffer, L"rb") != 0)
		return GameVersion::Unknown;
	
	fseek(file, 0, SEEK_END);
	const int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	std::vector<uint8_t> fileContent(fileSize);
	fread(fileContent.data(), fileContent.size(), 1, file);
	fclose(file);

	picosha2::hash256_one_by_one hasher;
	hasher.process(fileContent.begin(), fileContent.end());
	hasher.finish();

	std::vector<unsigned char> hash(picosha2::k_digest_size);
	hasher.get_hash_bytes(hash.begin(), hash.end());

	const std::string fileSha256 = picosha2::get_hash_hex_string(hasher);
	for (int i = 0; i < static_cast<int>(GameVersionHashes.size()); ++i)
	{
		if (GameVersionHashes[i] == fileSha256)
			return static_cast<GameVersion>(i);
	}

#ifdef _DEBUG
	OutputDebugStringA("Unknown game version : ");
	OutputDebugStringA(fileSha256.c_str());
	OutputDebugStringA("\n");
#endif

	return GameVersion::Unknown;
}