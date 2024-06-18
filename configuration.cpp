#include "framework.h"
#include "screensaver.h"
#include "registry.h"
#include <windows.h>
#include <string>
#include <vector>
#include <set>

Configuration::Configuration()
{
	auto path = GetRegistryPath();
	auto reg = Registry(HKEY_CURRENT_USER, path.c_str(), true);
	if (reg) {
		fontSize = reg["FontSize"];
		// Assuming Registry::operator[] returns a type that can be explicitly converted to std::string
		fontName = static_cast<std::string>(reg["FontName"]); // Explicitly cast to std::string
		clockFormat = static_cast<std::string>(reg["ClockFormat"]); // Explicitly cast to std::string
		gradientStartColor = reg["GradientStartColor"];
		gradientEndColor = reg["GradientEndColor"];
		fontColor = reg["FontColor"];
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
		reg["GradientStartColor"] = gradientStartColor;
		reg["GradientEndColor"] = gradientEndColor;
		reg["FontColor"] = fontColor;
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