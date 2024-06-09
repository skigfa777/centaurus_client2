#include "screenshot.h"
#include <stdio.h>
#include <windows.h>
#include <GdiPlus.h>
#include <atltime.h>
#include <string>

#include <opencv2/opencv.hpp>
#include "base64.h"

#pragma comment( lib, "gdiplus" )

using namespace std;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

string GetScreenshot();



int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    using namespace Gdiplus;
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return 0;
}

string GetScreenshot()
{
    using namespace Gdiplus;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    string base64 = "";

    {
        CString fileName = "screen.jpg";
        string readFile = "screen.jpg";

        HDC scrdc, memdc;
        HBITMAP membit;
        scrdc = ::GetDC(0);
        int Height = GetSystemMetrics(SM_CYSCREEN);
        int Width = GetSystemMetrics(SM_CXSCREEN);

        memdc = CreateCompatibleDC(scrdc);
        membit = CreateCompatibleBitmap(scrdc, Width, Height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(memdc, membit);
        BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

        Gdiplus::Bitmap bitmap(membit, NULL);
        CLSID clsid;
        GetEncoderClsid(L"image/jpeg", &clsid);
        bitmap.Save(fileName, &clsid);

        
        cv::Mat image = cv::imread(readFile);
        vector<uchar> buffer;
        buffer.resize(static_cast<size_t>(image.rows) * static_cast<size_t>(image.cols));
        cv::imencode(".jpg", image, buffer);


        base64 = base64_encode((unsigned const char*)buffer.data(), buffer.size());

        SelectObject(memdc, hOldBitmap);

        DeleteObject(memdc);

        DeleteObject(membit);

        ::ReleaseDC(0, scrdc);

    }

    GdiplusShutdown(gdiplusToken);

    return base64;
}

