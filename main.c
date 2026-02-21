#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "ppm.h"

PPM *g_image = NULL;
HBITMAP g_hBitmap = NULL;

int g_clientWidth = 0;
int g_clientHeight = 0;

HBITMAP create_bitmap_from_ppm(PPM *image) {
    if (!image) return NULL;

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image->width;
    bmi.bmiHeader.biHeight = -(LONG)image->height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    int rowSize = image->width * 3;
    uint8_t *pixels = malloc(rowSize * image->height);
    if (!pixels) return NULL;

    for (uint32_t y = 0; y < image->height; y++) {
        for (uint32_t x = 0; x < image->width; x++) {
            Pixel p = image->pixels[y][x];
            uint8_t *ptr = pixels + (y * rowSize + x * 3);
            ptr[0] = (uint8_t)p.b;
            ptr[1] = (uint8_t)p.g;
            ptr[2] = (uint8_t)p.r;
        }
    }

    HDC hdc = GetDC(NULL);
    void *bits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    ReleaseDC(NULL, hdc);

    if (!hBitmap) { free(pixels); return NULL; }

    memcpy(bits, pixels, rowSize * image->height);
    free(pixels);

    return hBitmap;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_SIZE:
            g_clientWidth = LOWORD(lParam);
            g_clientHeight = HIWORD(lParam);
            InvalidateRect(hwnd, NULL, TRUE); 
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (g_hBitmap) {
                HDC hMem = CreateCompatibleDC(hdc);
                SelectObject(hMem, g_hBitmap);

                // Stretch the bitmap to fit the current client area
                StretchBlt(
                    hdc, 0, 0, g_clientWidth, g_clientHeight,
                    hMem, 0, 0, g_image->width, g_image->height,
                    SRCCOPY
                );

                DeleteDC(hMem);
            }

            EndPaint(hwnd, &ps);
        } break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    if (lpCmdLine[0] == 0) {
        MessageBox(NULL, "Usage: program.exe <image.ppm>", "Error", MB_OK);
        return 0;
    }

    g_image = ppm_read_image(lpCmdLine);
    if (!g_image) {
        MessageBox(NULL, "Failed to load PPM image", "Error", MB_OK);
        return 0;
    }

    g_hBitmap = create_bitmap_from_ppm(g_image);
    if (!g_hBitmap) {
        ppm_free(g_image);
        MessageBox(NULL, "Failed to create bitmap", "Error", MB_OK);
        return 0;
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "PPMViewer";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    g_clientWidth = g_image->width;
    g_clientHeight = g_image->height;

    HWND hwnd = CreateWindow(
        "PPMViewer", "PPM Viewer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_clientWidth + 16, g_clientHeight + 39,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(g_hBitmap);
    ppm_free(g_image);

    return 0;
}