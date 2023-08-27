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
		"0ab61bada34783a684490858e2005bbd2e9a1b1353eaaad44337bf7abb773b72",	// V_20_10_18
		"d18cdf473c01afbd90ce1df0c3f3bb4a3d6c4ebfe4d792a035894c63a2790f8f",	// V_17_08_23
		"0cb0dc1d2f8d9b2e56de380c39dab3ed93fba10fc063a85e0c5b0623f1869345"	// V_17_08_23_Steamless
	};
	static_assert(GameVersionHashes.size() == static_cast<int>(GameVersion::Unknown));
}

GameVersion GetGameVersion()
{
	// Check game's executable SHA256
	wchar_t buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(nullptr, buffer, MAX_PATH);

	FILE* file = _wfopen(buffer, L"rb");
	if (file == nullptr)
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

	return GameVersion::Unknown;
}