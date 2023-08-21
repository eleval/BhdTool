#pragma once

// Value after V_ is the Game's Steam Depot's Manifest ID
// As seen on https://steamdb.info/depot/304242/manifests/
// _Steamlesss means it's a version from which Steam DRM has been removed.
enum class GameVersion
{
	V_3831846201811674141,
	V_7029559557684186662,
	V_7029559557684186662_Steamless,

	Unknown
};

GameVersion GetGameVersion();