#include "ScreenCapturer.h"
#include <iostream>

// ScreenCapturer 实现
int ScreenCapturer::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;
    UINT size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT i = 0; i < num; ++i)
    {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return i;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

ScreenCapturer::ScreenCapturer()
{
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

ScreenCapturer::~ScreenCapturer()
{
    GdiplusShutdown(gdiplusToken);
}

bool ScreenCapturer::SaveBitmapToFile(HBITMAP hBitmap, const std::wstring& filename)
{
    Bitmap bitmap(hBitmap, NULL);
    CLSID clsid;

    if (filename.find(L".png") != std::wstring::npos)
    {
        GetEncoderClsid(L"image/png", &clsid);
    }
    else
    {
        GetEncoderClsid(L"image/jpeg", &clsid);
    }

    Status status = bitmap.Save(filename.c_str(), &clsid, NULL);
    return status == Ok;
}

bool ScreenCapturer::CaptureFullScreen(const std::wstring& filename)
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    bool result = SaveBitmapToFile(hBitmap, filename);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return result;
}

bool ScreenCapturer::CaptureSpecificArea(int x, int y, int width, int height, const std::wstring& filename)
{
    if (width <= 0 || height <= 0)
    {
        std::cout << "区域尺寸无效！" << std::endl;
        return false;
    }

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

    bool result = SaveBitmapToFile(hBitmap, filename);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return result;
}

// InteractiveRegionCapturer 静态成员初始化
POINT InteractiveRegionCapturer::startPoint = { 0, 0 };
POINT InteractiveRegionCapturer::endPoint = { 0, 0 };
bool InteractiveRegionCapturer::isSelecting = false;
bool InteractiveRegionCapturer::hasStartedSelection = false;
bool InteractiveRegionCapturer::captureCompleted = false;
HHOOK InteractiveRegionCapturer::mouseHook = NULL;
HWND InteractiveRegionCapturer::overlayWindow = NULL;
HBITMAP InteractiveRegionCapturer::screenBitmap = NULL;
HDC InteractiveRegionCapturer::memDC = NULL;

// InteractiveRegionCapturer 实现
HWND InteractiveRegionCapturer::CreateOverlayWindow()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    UnregisterClass(L"ScreenCaptureOverlay_NoFlicker", hInstance);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ScreenCaptureOverlay_NoFlicker";
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassEx(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        L"ScreenCaptureOverlay_NoFlicker",
        L"",
        WS_POPUP,
        0, 0, sw, sh,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, 128, LWA_ALPHA);
    return hwnd;
}

LRESULT CALLBACK InteractiveRegionCapturer::OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, sw, sh);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        BitBlt(hdcMem, 0, 0, sw, sh, memDC, 0, 0, SRCCOPY);

        if (hasStartedSelection)
        {
            int x = min(startPoint.x, endPoint.x);
            int y = min(startPoint.y, endPoint.y);
            int w = abs(endPoint.x - startPoint.x);
            int h = abs(endPoint.y - startPoint.y);

            if (w > 0 && h > 0)
            {
                BLENDFUNCTION bf = { AC_SRC_OVER, 0, 128, 0 };
                HDC hdcTemp = CreateCompatibleDC(hdc);
                HBITMAP hbmTemp = CreateCompatibleBitmap(hdc, sw, sh);
                HBITMAP hbmOldTemp = (HBITMAP)SelectObject(hdcTemp, hbmTemp);

                HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
                RECT fullRect = { 0, 0, sw, sh };
                FillRect(hdcTemp, &fullRect, blackBrush);
                DeleteObject(blackBrush);

                AlphaBlend(hdcMem, 0, 0, sw, sh, hdcTemp, 0, 0, sw, sh, bf);
                BitBlt(hdcMem, x, y, w, h, memDC, x, y, SRCCOPY);

                HPEN blueDashPen = CreatePen(PS_DASH, 2, RGB(0, 120, 255));
                HPEN oldPen = (HPEN)SelectObject(hdcMem, blueDashPen);
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                Rectangle(hdcMem, x, y, x + w, y + h);

                SelectObject(hdcMem, oldPen);
                SelectObject(hdcMem, oldBrush);
                DeleteObject(blueDashPen);

                std::wstring text = std::to_wstring(w) + L" x " + std::to_wstring(h);
                SetBkMode(hdcMem, TRANSPARENT);
                SetTextColor(hdcMem, RGB(255, 255, 255));
                int textY = (y - 20 < 0) ? y + h + 5 : y - 20;
                TextOut(hdcMem, x, textY, text.c_str(), text.length());

                SelectObject(hdcTemp, hbmOldTemp);
                DeleteObject(hbmTemp);
                DeleteDC(hdcTemp);
            }
        }
        else
        {
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 128, 0 };
            HDC hdcTemp = CreateCompatibleDC(hdc);
            HBITMAP hbmTemp = CreateCompatibleBitmap(hdc, sw, sh);
            HBITMAP hbmOldTemp = (HBITMAP)SelectObject(hdcTemp, hbmTemp);

            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            RECT fullRect = { 0, 0, sw, sh };
            FillRect(hdcTemp, &fullRect, blackBrush);
            DeleteObject(blackBrush);

            AlphaBlend(hdcMem, 0, 0, sw, sh, hdcTemp, 0, 0, sw, sh, bf);

            SelectObject(hdcTemp, hbmOldTemp);
            DeleteObject(hbmTemp);
            DeleteDC(hdcTemp);

            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            std::wstring hint = L"点击并拖动鼠标选择截图区域";
            RECT textRect = { sw / 2 - 150, sh / 2 - 10, sw / 2 + 150, sh / 2 + 10 };
            DrawText(hdcMem, hint.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        BitBlt(hdc, 0, 0, sw, sh, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InteractiveRegionCapturer::UpdateOverlay()
{
    if (overlayWindow)
    {
        InvalidateRect(overlayWindow, NULL, TRUE);
        UpdateWindow(overlayWindow);
    }
}

void InteractiveRegionCapturer::CleanupResources()
{
    if (mouseHook)
    {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = NULL;
    }

    if (overlayWindow && IsWindow(overlayWindow))
    {
        DestroyWindow(overlayWindow);
        overlayWindow = NULL;
    }

    if (screenBitmap)
    {
        DeleteObject(screenBitmap);
        screenBitmap = NULL;
    }

    if (memDC)
    {
        DeleteDC(memDC);
        memDC = NULL;
    }

    isSelecting = false;
    hasStartedSelection = false;
    startPoint = { 0, 0 };
    endPoint = { 0, 0 };
}

LRESULT CALLBACK InteractiveRegionCapturer::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;

        switch (wParam)
        {
        case WM_LBUTTONDOWN:
            hasStartedSelection = true;
            startPoint = ms->pt;
            endPoint = ms->pt;
            UpdateOverlay();
            break;

        case WM_MOUSEMOVE:
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            {
                endPoint = ms->pt;
                UpdateOverlay();
            }
            break;

        case WM_LBUTTONUP:
            endPoint = ms->pt;
            isSelecting = false;
            PostQuitMessage(0);
            break;
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

bool InteractiveRegionCapturer::StartInteractiveCapture(const std::wstring& filename)
{
    CleanupResources();

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    screenBitmap = CreateCompatibleBitmap(hdcScreen, sw, sh);
    memDC = CreateCompatibleDC(hdcScreen);
    SelectObject(memDC, screenBitmap);
    BitBlt(memDC, 0, 0, sw, sh, hdcScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hdcScreen);

    isSelecting = true;
    hasStartedSelection = false;
    captureCompleted = false;
    startPoint = { 0, 0 };
    endPoint = { 0, 0 };

    overlayWindow = CreateOverlayWindow();
    if (!overlayWindow)
    {
        CleanupResources();
        return false;
    }

    ShowWindow(overlayWindow, SW_SHOW);
    UpdateWindow(overlayWindow);

    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
    if (!mouseHook)
    {
        CleanupResources();
        return false;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!isSelecting)
        {
            break;
        }
    }

    int x = min(startPoint.x, endPoint.x);
    int y = min(startPoint.y, endPoint.y);
    int w = abs(endPoint.x - startPoint.x);
    int h = abs(endPoint.y - startPoint.y);

    if (w <= 1 || h <= 1 || !hasStartedSelection)
    {
        CleanupResources();
        return false;
    }

    ScreenCapturer c;
    bool result = c.CaptureSpecificArea(x, y, w, h, filename);

    CleanupResources();
    return result;
}