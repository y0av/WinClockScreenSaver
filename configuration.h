#pragma once

// Class for managing screen saver configuration, and load/store value from/to
// Windows Registry. Put your screen saver variables here.
class Configuration
{
public:
	// The constructor loads data from registry when called
	Configuration();
	
	// Use this function for instantly write current configuration to registry
	void Commit();

	// Your screensaver variables, you can set the default value here too
	int fontSize = 80;
	std::string fontName = "Calibri"; // Default font name
	std::string clockFormat = "%H:%M:%S"; // 24-hour format
	COLORREF gradientStartColor = RGB(132, 112, 255);  // Light Slate Blue
	COLORREF gradientEndColor = RGB(255, 105, 180); // Hot Pink
	COLORREF fontColor = RGB(255, 255, 255); // White

private:
	static constexpr LPCSTR REGISTRY_PATH = "Software\\Microsoft\\Windows\\CurrentVersion\\Screensavers";
	
	// Get screensaver path on registry. If you want to rename, modify
	// IDS_APPNAME value in String Table from the resources.
	std::string GetRegistryPath();
};
