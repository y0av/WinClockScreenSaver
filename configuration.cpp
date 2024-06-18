#include "framework.h"
#include "screensaver.h"
#include "registry.h"
#include <windows.h>
#include <string>
#include <vector>
#include <set>

Configuration::Configuration()
{
	//std::vector<std::string> GetInstalledFonts();
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

//// Callback function for EnumFontFamiliesEx
//int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
//{
//	std::set<std::string>* fontNames = reinterpret_cast<std::set<std::string>*>(lParam);
//	fontNames->insert(lpelfe->elfLogFont.lfFaceName);
//	return 1; // Continue enumeration
//}
//
//std::vector<std::string> Configuration::GetInstalledFonts()
//{
//	std::set<std::string> uniqueFonts;
//
//	LOGFONT lf = { 0 };
//	lf.lfCharSet = DEFAULT_CHARSET;
//
//	HDC hdc = GetDC(NULL);
//	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&uniqueFonts, 0);
//	ReleaseDC(NULL, hdc);
//
//	std::vector<std::string> fonts(uniqueFonts.begin(), uniqueFonts.end());
//	return fonts;
//}

