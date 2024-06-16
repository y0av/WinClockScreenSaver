#include "framework.h"
#include "screensaver.h"
#include "registry.h"
#include <vector>

Configuration::Configuration()
{
	std::vector<std::string> GetInstalledFonts();
	auto path = GetRegistryPath();
	auto reg = Registry(HKEY_CURRENT_USER, path.c_str(), true);
	if (reg) {
		numStars      = reg["NumberOfStars"];
		fontSize = reg["FontSize"];
		// Assuming Registry::operator[] returns a type that can be explicitly converted to std::string
		fontName = static_cast<std::string>(reg["FontName"]); // Explicitly cast to std::string

	}
}

void Configuration::Commit()
{
	auto path = GetRegistryPath();
	auto reg = Registry(HKEY_CURRENT_USER, path.c_str(), false);
	if (reg) {
		reg["NumberOfStars"] = numStars;
		reg["FontSize"] = fontSize;
		reg["FontName"] = fontName; // Save font name to registry
	}
}

std::string Configuration::GetRegistryPath()
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	char app[MAX_PATH];
	LoadString(hInst, IDS_APPNAME, app, MAX_PATH);

	std::string path = std::string(REGISTRY_PATH);
	path += "\\";
	path += app;
	return path;
}

std::vector<std::string> Configuration::GetInstalledFonts()
{
	std::vector<std::string> fonts;
	// Get the number of installed fonts
	DWORD numFonts = GetFontData(NULL, 0, 0, NULL, 0);
	if (numFonts == GDI_ERROR) return fonts;

	// Allocate memory for the font names
	std::vector<char> fontData(numFonts);
	if (GetFontData(NULL, 0, 0, fontData.data(), numFonts) == GDI_ERROR) return fonts;

	// Parse the font names
	const char* ptr = fontData.data();
	while (ptr < fontData.data() + numFonts) {
		fonts.push_back(ptr);
		ptr += strlen(ptr) + 1;
	}
	return fonts;
}
