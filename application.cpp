#include "framework.h"
#include "screensaver.h"
using namespace std;

Application::Application(HWND hWnd, Configuration* config)
	: hWnd(hWnd), config(config)
{

#ifdef _DEBUG
	running = true;
#endif

	// Initialize stars and timer
	Initialize();
}

void Application::Deinitialize()
{
	KillTimer(hWnd, TIMER_ID);
}

void Application::Initialize()
{
	SetTimer(hWnd, TIMER_ID, config->fontSize, nullptr);


	// Use font from configuration
	hFont = CreateFont(
		config->fontSize, // Height of font
		0, // Width of font
		0, // Escapement
		0, // Orientation
		FW_BOLD, // Font weight
		FALSE, // Italic
		FALSE, // Underline
		FALSE, // Strikeout
		ANSI_CHARSET, // Character set
		OUT_DEFAULT_PRECIS, // Output precision
		CLIP_DEFAULT_PRECIS, // Clipping precision
		DEFAULT_QUALITY, // Output quality
		DEFAULT_PITCH | FF_DONTCARE, // Pitch and family
		config->fontName.c_str()); // Font name


	// Clear last rendered screen (in debug mode)
	InvalidateRect(hWnd, nullptr, TRUE);
}

void Application::SetCenterPosition(int x, int y)
{
	centerX = x / 2;
	centerY = y / 2;
}

COLORREF MixColors(COLORREF color1, COLORREF color2, double ratio) {
	BYTE r = GetRValue(color1) * (1 - ratio) + GetRValue(color2) * ratio;
	BYTE g = GetGValue(color1) * (1 - ratio) + GetGValue(color2) * ratio;
	BYTE b = GetBValue(color1) * (1 - ratio) + GetBValue(color2) * ratio;
	return RGB(r, g, b);
}


void Application::DrawBackground(HDC hDC)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	// Define main colors for the gradient
	COLORREF color1 = RGB(132, 112, 255); // Light Slate Blue
	COLORREF color2 = RGB(255, 105, 180); // Hot Pink

	// Calculate the center and maximum radius for the gradient circles
	int centerX = width / 2;
	int centerY = height / 2;
	int maxRadius = (int)sqrt(centerX * centerX + centerY * centerY);

	// Draw concentric circles with varying colors
	for (int radius = maxRadius; radius > 0; radius -= 5) { // Adjust step for performance vs. quality
		double ratio = (double)radius / maxRadius;

		COLORREF mixedColor = MixColors(color1, color2, ratio);
		// mix random color
		//COLORREF mixedColor = RGB(rand() % 256, rand() % 256, rand() % 256);

		HBRUSH hBrush = CreateSolidBrush(mixedColor);
		HPEN hPen = CreatePen(PS_SOLID, 1, mixedColor);
		SelectObject(hDC, hBrush);
		SelectObject(hDC, hPen);

		// Draw the circle
		Ellipse(hDC, centerX - radius, centerY - radius, centerX + radius, centerY + radius);

		DeleteObject(hBrush);
		DeleteObject(hPen);
	}

	// Draw the clock at the center of the screen
	// Set the background color to black
	SetBkColor(hDC, RGB(0, 0, 0));
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 255, 255)); // Assuming WHITE is defined as RGB(255, 255, 255)
}



void Application::Draw(HDC hDC)
{
	// Get current time
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);


	// Format time as HH:MM:SS using localtime_s for safer conversion
	char timeStr[9]; // HH:MM:SS\0
	std::tm tmResult;
	localtime_s(&tmResult, &in_time_t);
	std::strftime(timeStr, sizeof(timeStr), /* use clock format saved in config */ 
		 config->clockFormat.c_str() 		//"%H:%M:%S"
		, &tmResult);

	


	SelectObject(hDC, hFont);
	SIZE textSize;
	GetTextExtentPoint32(hDC, timeStr, strlen(timeStr), &textSize);

	// Calculate the starting position to center the text
	int startX = centerX - (textSize.cx / 2);
	int startY = centerY - (textSize.cy / 2);

	TextOut(hDC, startX, startY, timeStr, strlen(timeStr));
}


void Application::Update()
{
	
}

void Application::Loop()
{
#ifdef _DEBUG
	if (running) {
#endif
		Update();

		// Create an off-screen DC
		HDC hdcScreen = GetDC(hWnd);
		HDC hdcBuffer = CreateCompatibleDC(hdcScreen);
		RECT rect;
		GetClientRect(hWnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		HBITMAP hbmBuffer = CreateCompatibleBitmap(hdcScreen, width, height);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

		// Fill the buffer with black (or any other background color)
		FillRect(hdcBuffer, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		// Perform all drawing operations on the off-screen DC, including background
		DrawBackground(hdcBuffer); // Draw the gradient background first
		Draw(hdcBuffer); // Then draw the rest of the scene

		// Copy the off-screen buffer to the screen
		BitBlt(hdcScreen, 0, 0, width, height, hdcBuffer, 0, 0, SRCCOPY);

		// Clean up
		SelectObject(hdcBuffer, hbmOld);
		DeleteObject(hbmBuffer);
		DeleteDC(hdcBuffer);
		ReleaseDC(hWnd, hdcScreen);
#ifdef _DEBUG
	}
#endif
}
