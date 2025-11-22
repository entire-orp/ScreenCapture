#ifndef SCREENCAPTURER_H
#define SCREENCAPTURER_H

#include <windows.h>
#include <gdiplus.h>
#include <string>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace Gdiplus;

class ScreenCapturer
{
private:
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

public:
    ScreenCapturer();
    ~ScreenCapturer();

    bool SaveBitmapToFile(HBITMAP hBitmap, const std::wstring& filename);
    bool CaptureFullScreen(const std::wstring& filename);
    bool CaptureSpecificArea(int x, int y, int width, int height, const std::wstring& filename);
};

class InteractiveRegionCapturer
{
private:
    static POINT startPoint, endPoint;
    static bool isSelecting;
    static bool hasStartedSelection;
    static bool captureCompleted;
    static HHOOK mouseHook;
    static HWND overlayWindow;
    static HBITMAP screenBitmap;
    static HDC memDC;

    static HWND CreateOverlayWindow();
    static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void UpdateOverlay();
    static void CleanupResources();
    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    static bool StartInteractiveCapture(const std::wstring& filename);
};

#endif // SCREENCAPTURER_H