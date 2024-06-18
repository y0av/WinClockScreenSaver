#include "framework.h"
#include "screensaver.h"
#include <set>
#include <windows.h>
#include <commdlg.h>
#ifndef _DEBUG
#include <ScrnSave.h>
#endif

#ifdef _MSC_VER
#ifndef _DEBUG
#pragma comment(lib, "scrnsave.lib")   // Microsoft Visual C++ library for creating Screen saver application
#endif
#pragma comment(lib, "comctl32.lib")   // Windows Common Control Library, for the controls on configure dialog
#endif

static Application app;
static Configuration config;
std::string lastFontName;

// Use this function if you want to register new window class for the configure
// dialog, with the hModule parameter as HINSTANCE in CreateWindow function.
// Must be return TRUE if screen saver has configure dialog. FALSE otherwise
BOOL WINAPI RegisterDialogClasses(HANDLE hModule)
{
	return TRUE;
}


// Callback function for EnumFontFamiliesEx
int CALLBACK EnumFontsProc(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
{
	// if current font name is different from the last font name
	if (lastFontName != lpelfe->lfFaceName) {
		// add the font name to the combo box
		HWND hwndCombo = (HWND)lParam;
		SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
		// save the font name to a variable
		lastFontName = lpelfe->lfFaceName;
	}
	return 1; // Continue enumeration
}

// Handle messages for the screen saver's configuration dialog box. The dialog 
// template in resource file must have DLG_SCRNSAVECONFIGURE identifier.
// Return TRUE if the function successfully process the message.
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndFormatComboBox;
	HWND hwndFontNameComboBox;
	int formatSelectedIndex;
	int fontSelectedIndex;

	switch (uMsg) {
		case WM_INITDIALOG: {
			// Initialize the configuration
			config = Configuration();



			// Set range of the sliders
			SendDlgItemMessage(hDlg, IDC_SLIDER_NUMOFSTARS, TBM_SETRANGE, FALSE, MAKELPARAM(50, 1000));
			SendDlgItemMessage(hDlg, IDC_SLIDER_INTERVAL, TBM_SETRANGE, FALSE, MAKELPARAM(1, 300));

			// Set position of the sliders
			SendDlgItemMessage(hDlg, IDC_SLIDER_INTERVAL, TBM_SETPOS, TRUE, config.fontSize);


			// Set value of the slider texts
			SetDlgItemInt(hDlg, IDC_TEXT_INTERVAL, config.fontSize, FALSE);

			HDC hdc = GetDC(hDlg); // Corrected variable name here
			LOGFONT lf = { 0 };
			lf.lfCharSet = DEFAULT_CHARSET;

			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontsProc, (LPARAM)GetDlgItem(hDlg, IDC_COMBO_FONTNAME), 0);

			// Populate the clock format combobox
			HWND hwndClockFormatCombo = GetDlgItem(hDlg, IDC_COMBO_CLOCKFORMAT);
			SendMessage(hwndClockFormatCombo, CB_ADDSTRING, 0, (LPARAM)"%H:%M:%S");
			SendMessage(hwndClockFormatCombo, CB_ADDSTRING, 0, (LPARAM)"%H:%M");
			//SendMessage(hwndClockFormatCombo, CB_ADDSTRING, 0, (LPARAM)"%I:%M:%S %p"); // not sure why but this one is not working
			SendMessage(hwndClockFormatCombo, CB_ADDSTRING, 0, (LPARAM)"%I:%M %p");    // 12-hour format without seconds
			SendMessage(hwndClockFormatCombo, CB_SETCURSEL, 0, 0); // Selects the first format by default
			// set the combo box to the previously selected format
			for (int i = 0; i < SendMessage(hwndClockFormatCombo, CB_GETCOUNT, 0, 0); i++) {
				int textLen = SendMessage(hwndClockFormatCombo, CB_GETLBTEXTLEN, (WPARAM)i, 0);
				std::vector<char> buffer(textLen + 1);
				SendMessage(hwndClockFormatCombo, CB_GETLBTEXT, (WPARAM)i, (LPARAM)buffer.data());
				std::string format(buffer.begin(), buffer.end() - 1);
				if (format == config.clockFormat) {
					SendMessage(hwndClockFormatCombo, CB_SETCURSEL, i, 0);
					break;
				}
			}
			// set the combo box to the previously selected font
			hwndFontNameComboBox = GetDlgItem(hDlg, IDC_COMBO_FONTNAME);
			for (int i = 0; i < SendMessage(hwndFontNameComboBox, CB_GETCOUNT, 0, 0); i++) {
				int textLen = SendMessage(hwndFontNameComboBox, CB_GETLBTEXTLEN, (WPARAM)i, 0);
				std::vector<char> buffer(textLen + 1);
				SendMessage(hwndFontNameComboBox, CB_GETLBTEXT, (WPARAM)i, (LPARAM)buffer.data());
				std::string font(buffer.begin(), buffer.end() - 1);
				if (font == config.fontName) {
					SendMessage(hwndFontNameComboBox, CB_SETCURSEL, i, 0);
					break;
				}
			}

			ReleaseDC(hDlg, hdc); // Corrected variable name here

			return TRUE;
		}
		case WM_HSCROLL: {
			// Link slider texts value with sliders
			int interval = SendDlgItemMessage(hDlg, IDC_SLIDER_INTERVAL,   TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_TEXT_INTERVAL, interval, FALSE);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BUTTON_DEFAULT:
					// Reset all configurations to default value
					SendDlgItemMessage(hDlg, IDC_SLIDER_INTERVAL,   TBM_SETPOS, TRUE, 20);
					SetDlgItemInt(hDlg, IDC_TEXT_INTERVAL,   20,  FALSE);
					SendDlgItemMessage(hDlg, IDC_COMBO_FONTNAME, CB_SETCURSEL, 0, 0);

					return TRUE;
				case IDOK:
					config.fontSize = SendDlgItemMessage(hDlg, IDC_SLIDER_INTERVAL,   TBM_GETPOS, 0, 0);

					// Get the selected clock format
					hwndFormatComboBox = GetDlgItem(hDlg, IDC_COMBO_CLOCKFORMAT);
                    formatSelectedIndex = SendMessage(hwndFormatComboBox, CB_GETCURSEL, 0, 0);

					if (formatSelectedIndex != CB_ERR) {
						// Get the length of the selected item's text
						int formatTextLen = SendMessage(hwndFormatComboBox, CB_GETLBTEXTLEN, (WPARAM)formatSelectedIndex, 0);
						// Allocate buffer for the text (+1 for the null terminator)
						std::vector<char> buffer(formatTextLen + 1);
						// Retrieve the selected item's text
						SendMessage(hwndFormatComboBox, CB_GETLBTEXT, (WPARAM)formatSelectedIndex, (LPARAM)buffer.data());
						// Convert to std::string (or std::wstring if using Unicode)
						std::string selectedFormat(buffer.begin(), buffer.end() - 1); // -1 to exclude the null terminator
						// Now you can use selectedFormat as needed
						config.clockFormat = selectedFormat;
					}

					// do the same like we fetched the clock format to the font name
										// Get the selected font name
					hwndFontNameComboBox = GetDlgItem(hDlg, IDC_COMBO_FONTNAME);
					fontSelectedIndex = SendMessage(hwndFontNameComboBox, CB_GETCURSEL, 0, 0);
					if (fontSelectedIndex != CB_ERR) {
						// Get the length of the selected item's text
						int fontTextLen = SendMessage(hwndFontNameComboBox, CB_GETLBTEXTLEN, (WPARAM)fontSelectedIndex, 0);
						// Allocate buffer for the text (+1 for the null terminator)
						std::vector<char> buffer(fontTextLen + 1);
						// Retrieve the selected item's text
						SendMessage(hwndFontNameComboBox, CB_GETLBTEXT, (WPARAM)fontSelectedIndex, (LPARAM)buffer.data());
						// Convert to std::string (or std::wstring if using Unicode)
						std::string selectedFont(buffer.begin(), buffer.end() - 1); // -1 to exclude the null terminator
						// Now you can use selectedFont as needed
						config.fontName = selectedFont;
					}


					config.Commit();
					// Pass through to next case
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				case (IDC_COLOR_GRADIENTSTARTCOLOR):
					{
					CHOOSECOLOR cc = { sizeof(cc) };
					COLORREF acrCustClr[16];
					cc.lpCustColors = acrCustClr;
					cc.rgbResult = config.gradientStartColor;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc)) {
						config.gradientStartColor = cc.rgbResult;
					}
					return TRUE;
				}
				case (IDC_COLOR_GRADIENTENDCOLOR):
					{
					CHOOSECOLOR cc = { sizeof(cc) };
					COLORREF acrCustClr[16];
					cc.lpCustColors = acrCustClr;
					cc.rgbResult = config.gradientEndColor;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc)) {
						config.gradientEndColor = cc.rgbResult;
					}
					return TRUE;
				}
				case (IDC_COLOR_FONTCOLOR):
					{
					CHOOSECOLOR cc = { sizeof(cc) };
					COLORREF acrCustClr[16];
					cc.lpCustColors = acrCustClr;
					cc.rgbResult = config.fontColor;
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc)) {
						config.fontColor = cc.rgbResult;
					}
					return TRUE;
				}

			}
			break;
	}
	return FALSE;
}

// The screensaver's window procedure, like lpfnWndProc callback in WNDCLASS struct for creating
// normal Windows application. Use DefScreenSaverProc instead of DefWindowProc for default message
// processing. All rendering process will be processed in this function.
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_CREATE:
			app = Application(hWnd, &config);
			break;
		case WM_DESTROY:
			app.Deinitialize();
			break;
		case WM_SIZE:
			app.SetCenterPosition(LOWORD(lParam), HIWORD(lParam));
			break;
		// WM_PAINT case can be removed if you not using GDI renderer
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}
		// WM_TIMER case can be removed if you use high resolution timer (QPC and QPF combination)
		case WM_TIMER:
			if (wParam == Application::TIMER_ID) {
				app.Loop();
			}
			break;
		default:
#ifdef _DEBUG
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
#else
			return DefScreenSaverProc(hWnd, uMsg, wParam, lParam);
#endif
	}

	return 0;
}

// Create a windowed screen saver, since output .scr file is not debugable (will stuck
// on the main screen and freezing if breakpoint was triggered).
#ifdef _DEBUG
constexpr uint32_t IDM_CONFIGURE = 11001;
constexpr uint32_t IDM_EXIT = 11002;

static LRESULT WINAPI DebugScreenSaverProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE: {
            HMENU hMenuBar = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenu(hFileMenu, MF_STRING, IDM_CONFIGURE, "&Configure");
            AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
            AppendMenu(hFileMenu, MF_STRING, IDM_EXIT, "&Exit");
            AppendMenu(hMenuBar, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), "&File");
            SetMenu(hWnd, hMenuBar);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_CONFIGURE: {
                    HINSTANCE hInst = GetModuleHandle(nullptr);
                    if (!RegisterDialogClasses(hInst)) {
                        break;
                    }
                    app.Pause();
                    DialogBox(hInst, MAKEINTRESOURCE(DLG_SCRNSAVECONFIGURE), hWnd, reinterpret_cast<DLGPROC>(ScreenSaverConfigureDialog));
                    app.Unpause();
                    app.Reload();
                    break;
                }
                case IDM_EXIT:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;
    }
    return ScreenSaverProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdCount)
{
	InitCommonControls();

	constexpr LPCSTR WINDOW_CLASS = "DebugScreenSaverWindow";
	WNDCLASS wc = {};
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SCREENSAVER));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = WINDOW_CLASS;
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.hInstance = hInst;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = DebugScreenSaverProc;
	
	// Set window size here, and window position at center of screen
	constexpr UINT STYLE = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
	constexpr UINT EX_STYLE = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	constexpr int WIDTH = 800;
	constexpr int HEIGHT = 600;
	const int x = (GetSystemMetrics(SM_CXSCREEN) - WIDTH) / 2;
	const int y = (GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2;

	// Lazy window initialization (no error handling)
	RegisterClass(&wc);
	CreateWindowEx(EX_STYLE, WINDOW_CLASS, "Screen Saver Preview", STYLE, x, y, WIDTH, HEIGHT, nullptr, nullptr, hInst, nullptr);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
#endif
