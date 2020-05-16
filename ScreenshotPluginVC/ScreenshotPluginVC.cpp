#include "plugin.h"

using namespace plugin;

#include <windows.h>
#include <filesystem>
#pragma comment(lib, "winmm.lib")
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

#include <string>
#include <stdio.h>
#include <time.h>
#include <game_vc/CHud.h>

#pragma warning(disable : 4996) 

void take_screenshot(char* BmpName)
{
	HWND DesktopHwnd = GetDesktopWindow();
	RECT DesktopParams;
	HDC DevC = GetDC(DesktopHwnd);
	GetWindowRect(DesktopHwnd, &DesktopParams);
	DWORD Width = DesktopParams.right - DesktopParams.left;
	DWORD Height = DesktopParams.bottom - DesktopParams.top;

	DWORD FileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBTRIPLE) + 1 * (Width * Height * 4));
	char* BmpFileData = (char*)GlobalAlloc(0x0040, FileSize);

	PBITMAPFILEHEADER BFileHeader = (PBITMAPFILEHEADER)BmpFileData;
	PBITMAPINFOHEADER  BInfoHeader = (PBITMAPINFOHEADER)&BmpFileData[sizeof(BITMAPFILEHEADER)];

	BFileHeader->bfType = 0x4D42; // BM
	BFileHeader->bfSize = sizeof(BITMAPFILEHEADER);
	BFileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
	BInfoHeader->biPlanes = 1;
	BInfoHeader->biBitCount = 24;
	BInfoHeader->biCompression = BI_RGB;
	BInfoHeader->biHeight = Height;
	BInfoHeader->biWidth = Width;

	RGBTRIPLE* Image = (RGBTRIPLE*)&BmpFileData[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
	RGBTRIPLE color;

	HDC CaptureDC = CreateCompatibleDC(DevC);
	HBITMAP CaptureBitmap = CreateCompatibleBitmap(DevC, Width, Height);
	SelectObject(CaptureDC, CaptureBitmap);
	BitBlt(CaptureDC, 0, 0, Width, Height, DevC, 0, 0, SRCCOPY | CAPTUREBLT);
	GetDIBits(CaptureDC, CaptureBitmap, 0, Height, Image, (LPBITMAPINFO)BInfoHeader, DIB_RGB_COLORS);

	DWORD Junk;
	HANDLE FH = CreateFileA(BmpName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	WriteFile(FH, BmpFileData, FileSize, &Junk, 0);
	CloseHandle(FH);
	GlobalFree(BmpFileData);
}

class VCScreenshotPlugin {
public:
	VCScreenshotPlugin() {
		Events::gameProcessEvent += [] {
			if (KeyPressed(0x77)) { // f8
				CHAR usr_doucment_path[MAX_PATH]{};

				HRESULT doucmentsResult = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, usr_doucment_path);

				time_t     now = time(0);
				struct tm  tstruct;
				char       time_buffer[80];
				tstruct = *localtime(&now);

				char screenshot_store_bufer[256];

				strftime(time_buffer, sizeof(time_buffer), "%Y_%m_%d_%H_%S", &tstruct);

				snprintf(screenshot_store_bufer, sizeof screenshot_store_bufer, "%s\\GTA Vice City User Files\\screenshots\\gtavc_%s.png", usr_doucment_path, time_buffer);
				std::filesystem::create_directory(strcat(usr_doucment_path, "\\GTA Vice City User Files\\screenshots"));

				take_screenshot(screenshot_store_bufer);

				CHud::SetHelpMessage("Screenshot taken", true, false);

			}
		};
	}
} vCScreenshotPlugin;
