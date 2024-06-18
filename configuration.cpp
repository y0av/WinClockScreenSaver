#include "framework.h"
#include "screensaver.h"
#include "registry.h"
#include <vector>
#include <set>

Configuration::Configuration()
{
	std::vector<std::string> GetInstalledFonts();
	auto path = GetRegistryPath();
	auto reg = Registry(HKEY_CURRENT_USER, path.c_str(), true);
	if (reg) {
		fontSize = reg["FontSize"];
		// Assuming Registry::operator[] returns a type that can be explicitly converted to std::string
		fontName = static_cast<std::string>(reg["FontName"]); // Explicitly cast to std::string
		clockFormat = static_cast<std::string>(reg["ClockFormat"]); // Explicitly cast to std::string

	}
}

void Configuration::Commit()
{
	auto path = GetRegistryPath();
	auto reg = Registry(HKEY_CURRENT_USER, path.c_str(), false);
	if (reg) {
		reg["FontSize"] = fontSize;
		reg["FontName"] = fontName; // Save font name to registry
		reg["ClockFormat"] = clockFormat; // Save clock format to registry
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
	std::set<std::string> uniqueFonts; // Use std::set to store unique font names
	// Get the number of installed fonts
	DWORD numFonts = GetFontData(NULL, 0, 0, NULL, 0);
	if (numFonts == GDI_ERROR) return std::vector<std::string>(uniqueFonts.begin(), uniqueFonts.end());

	// Allocate memory for the font names
	std::vector<char> fontData(numFonts);
	if (GetFontData(NULL, 0, 0, fontData.data(), numFonts) == GDI_ERROR) return std::vector<std::string>(uniqueFonts.begin(), uniqueFonts.end());

	// Parse the font names
	const char* ptr = fontData.data();
	while (ptr < fontData.data() + numFonts) {
		uniqueFonts.insert(ptr); // Insert into set to ensure uniqueness
		ptr += strlen(ptr) + 1;
	}

	// Convert set to vector to maintain original method signature
	std::vector<std::string> fonts(uniqueFonts.begin(), uniqueFonts.end());
	return fonts;
}

