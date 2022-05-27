/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   EcoFormDialog
 * </сводка>
 *
 * <описание>
 *   Данный исходный файл является точкой входа
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */

#include "windows.h"
#include "commctrl.h"
#include "commdlg.h"
#include "resource.h"
#pragma comment(lib, "user32")
#pragma comment(lib, "comctl32.lib")

/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoFileSystemManagement1.h"
#include "EcoFormDialog.h"

/* Указатель на интерфейс для работы с BMP файлом */
extern IEcoImageBMP3File* g_pIBMP1;
extern IEcoImageBMP3File* g_pIBMP2;
extern IEcoImageBMP3File* g_pIBMPMean;
extern IEcoImageBMP3File** train_images;

/*
 *
 * <сводка>
 *   Функция DialogProc
 * </сводка>
 *
 * <описание>
 *   Функция DialogProc
 * </описание>
 *
 */
BOOL CALLBACK DrawImage(HWND hwndPictureBox, IEcoImageBMP3File* pIBMPFile) { 
    BITMAPINFO xBmi = {0};
    byte_t* buffer = NULL;
    uint32_t size = 0;
    HBITMAP hBmp;
    HDC winDC;
    HDC memoryDC;
    HBITMAP hOldBitmap;
    RECT rect = { 0 };

    winDC = GetDC(hwndPictureBox);
    xBmi.bmiHeader.biSize = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biSize;
    xBmi.bmiHeader.biSizeImage = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biSizeImage;
    xBmi.bmiHeader.biBitCount = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biBitCount;
    xBmi.bmiHeader.biHeight = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biHeight;
    xBmi.bmiHeader.biWidth = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biWidth;
    xBmi.bmiHeader.biPlanes = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biPlanes;
    xBmi.bmiHeader.biCompression   = BI_RGB;
    hBmp = CreateDIBSection(GetDC(hwndPictureBox), &xBmi, DIB_RGB_COLORS, (void**) &buffer, 0, 0);
    size = pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biWidth * (pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biBitCount / 8) * pIBMPFile->pVTbl->get_InfoHeader(pIBMPFile)->biHeight;
    pIBMPFile->pVTbl->ReadBitmapData(pIBMPFile, buffer, &size);
    memoryDC = CreateCompatibleDC(GetDC(hwndPictureBox));
    hOldBitmap = (HBITMAP)SelectObject(memoryDC, hBmp);
    GetWindowRect(hwndPictureBox, &rect);
    BitBlt(winDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memoryDC, 0, 0, SRCCOPY);
    /* Освобождение ресурсов */
    SelectObject(memoryDC, hOldBitmap);
    DeleteDC(memoryDC);
    ReleaseDC(hwndPictureBox, memoryDC);
    ReleaseDC(hwndPictureBox, winDC);
}


/*
 *
 * <сводка>
 *   Функция DialogProc
 * </сводка>
 *
 * <описание>
 *   Функция DialogProc
 * </описание>
 *
 */
BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) { 
    HWND hwndPictureBox;
    HDC hdc;
    PAINTSTRUCT ps;
    int32_t i = 0;

    switch (message) { 
        case WM_CREATE:
           CreateWindow(TEXT("STATIC"), TEXT("Image 1"), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 10, 14, 120, 26, hDlg, (HMENU)IDC_STATIC_IMAGE1, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);
           CreateWindow(TEXT("STATIC"), NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 30, 100, 120, hDlg, (HMENU)IDC_BITMAP_IMAGE1, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);

           CreateWindow(TEXT("STATIC"), TEXT("Image 2"), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 140, 14, 120, 26, hDlg, (HMENU)IDC_STATIC_IMAGE2, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);
           CreateWindow(TEXT("STATIC"), NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 140, 30, 100, 120, hDlg, (HMENU)IDC_BITMAP_IMAGE2, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);

           CreateWindow(TEXT("STATIC"), TEXT("Phase 2"), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP , 270, 14, 120, 26, hDlg, (HMENU)IDC_STATIC_PHASE2, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);
           CreateWindow(TEXT("STATIC"), NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 270, 30, 100, 120, hDlg, (HMENU)IDC_BITMAP_PHASE2, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), 0);
        return TRUE;
        case WM_PAINT:
            hdc = BeginPaint(hDlg, &ps);
            if (train_images[1] != 0) {
                hwndPictureBox = GetDlgItem(hDlg, IDC_BITMAP_IMAGE1);
                DrawImage(hwndPictureBox, train_images[1]);
            }
            if (train_images[1] != 0) {
                hwndPictureBox = GetDlgItem(hDlg, IDC_BITMAP_IMAGE2);
                DrawImage(hwndPictureBox, train_images[1]);
            }
            if (g_pIBMPMean != 0) {
                hwndPictureBox = GetDlgItem(hDlg, IDC_BITMAP_PHASE2);
                DrawImage(hwndPictureBox, g_pIBMPMean);
            }
            GdiFlush();
            EndPaint(hDlg, &ps);
        break;
        case WM_COMMAND: 
            switch (LOWORD(wParam)) { 
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, wParam); 
                return TRUE; 
            }
        break;
        case WM_CLOSE:
            //if (MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"), MB_ICONQUESTION | MB_YESNO) == IDYES) {
                DestroyWindow(hDlg);
            //}
            return TRUE;

        case WM_DESTROY:
            PostQuitMessage(0);
            return TRUE;
        default:
            return DefWindowProc(hDlg, message, wParam, lParam);
    }
    return FALSE;
}

/*
 *
 * <сводка>
 *   Функция DialogPreview
 * </сводка>
 *
 * <описание>
 *   Функция DialogPreview
 * </описание>
 *
 */
int16_t DialogPreview() {
    int16_t result = -1;
    HWND hDlg;
    WNDCLASS wndClass = {0};
    CREATESTRUCT xCS = {0};
    MSG msg;
    BOOL ret;
    RECT rectScreen;
    int32_t x = 0;
    int32_t y = 0;
    int32_t nCmdShow = 0;

    /* вычисление координат центра экрана */
    GetWindowRect(GetDesktopWindow(), &rectScreen);
    x = rectScreen.right / 2 - 150;
    y = rectScreen.bottom / 2 - 75;

    /* Регистрация класса */
    wndClass.lpfnWndProc = (WNDPROC)DialogProc;
    wndClass.style = CS_HREDRAW|CS_VREDRAW;
    wndClass.hInstance =  GetModuleHandle(0);
    wndClass.lpszClassName = TEXT("Eco ML Preview");
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;

    RegisterClass(&wndClass);
    FreeConsole();
    /* Инициализация элементов управления и создание диалогового окна */
    InitCommonControls();
    hDlg = CreateWindow(TEXT("Eco ML Preview"), TEXT("Eco ML Preview"), WS_DLGFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU, x, y, 670, 510, NULL, NULL,  GetModuleHandle(0), NULL);
    ShowWindow(hDlg, SW_SHOWDEFAULT);

    /* Цикл обработки Windows сообщений */
    while   ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
        if(ret == -1)
            return -1;

        if  (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return result;
}

