#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <mshtmcid.h>
#include <tchar.h>


#pragma comment(lib, "comctl32.lib") 
// linking with Comctl32.lib for the toolbar
// according to: https://docs.microsoft.com/en-us/answers/questions/302271/when-compiling-the-example-from-msdn-the-following.html

#define FILE_NEW 1
#define FILE_OPEN 2
#define FILE_COLOR 3
#define FILE_SAVE 4
#define FILE_FLIP_HORZ 5
#define FILE_FLIP_VERT 6
#define FILE_BW 7
#define FILE_QUIT 8

#define numToolbarButtons 4

#define create_menus(menu)\
AppendMenuW(menu, MF_STRING, FILE_NEW, L"&New");\
AppendMenuW(menu, MF_STRING, FILE_OPEN, L"&Open");\
AppendMenuW(menu, MF_STRING, FILE_COLOR, L"&Color Picker");\
AppendMenuW(menu, MF_SEPARATOR, 0, NULL);\
AppendMenuW(menu, MF_STRING, FILE_QUIT, L"&Quit");\

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND);
void OpenDialog(HWND);
void SaveDialog(HWND);
HWND CreateSimpleToolbar(HWND);

HWND ghwndEdit;

// for image itself
TCHAR pathName[MAX_PATH]; // path name = the current file
TCHAR tmpPath[MAX_PATH]; // tmp path = tmp file path

// for toolbar func
HIMAGELIST g_hImageList = NULL;
const int ToolbarBitmapSize = 16;

// HANDLE bitmap
static HBITMAP hBitmap;
static HBITMAP new_image;

// Variables for the temp file
DWORD dwRetVal = 0;
UINT uRetVal = 0;
TCHAR lpTempPathBuffer[MAX_PATH];
HANDLE hFile = INVALID_HANDLE_VALUE;
TCHAR szTempFileName[MAX_PATH];

// Building this way (efficiency 100)
#include "utils.c"
#include "parser.c"
#include "functions.c"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR lpCmdLine, int nCmdShow) {

    MSG  msg;
    WNDCLASSW wc = { 0 };

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpszClassName = L"Very cool editor";
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);

    RegisterClassW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Very cool editor",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 280, 220, NULL, NULL, hInstance, NULL);

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }

    return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {


    HDC hdc;
    PAINTSTRUCT ps;
    BITMAP bitmap;
    HDC hdcMem;
    HGDIOBJ oldBitmap;
    RECT rc;

    POINT point;
    HMENU hMenuPopup;

    switch (msg) {

    case WM_CREATE:

        CreateSimpleToolbar(hwnd);
        AddMenus(hwnd);

        dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
            lpTempPathBuffer); // buffer for path 
        if (dwRetVal > MAX_PATH || (dwRetVal == 0))
        {
            return (2);
        }

        //  Generates a temporary file name. 
        uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
            TEXT("DEMO"),     // temp file name prefix 
            0,                // create unique name 
            szTempFileName);  // buffer for name 
        if (uRetVal == 0)
        {
            return (3);
        }
        else
        {
            strcpy(tmpPath, szTempFileName);
        }

        break;

    case WM_PAINT:

        hBitmap = (HBITMAP)LoadImageA(NULL, tmpPath,
            IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        hdc = BeginPaint(hwnd, &ps);

        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);

        GetObject(hBitmap, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 5, 47, bitmap.bmWidth, bitmap.bmHeight,
            hdcMem, 0, 0, SRCCOPY);
        //47 so toolbar & bmp wont overlap

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

        EndPaint(hwnd, &ps);

        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case FILE_NEW:
        case FILE_OPEN:
            if (wParam == FILE_NEW || wParam == FILE_OPEN) {
                OpenDialog(hwnd);
            }
            break;

        case FILE_COLOR: {
            MessageBeep(MB_OK); //TODO: draw pixels
            break;
        }

        case FILE_SAVE: {
            if (pathName[0] != NULL)
            {
                if (checkFileIntegrity(pathName) == 0) {
                    SaveDialog(hwnd);
                }
                else {
                    noFileSelected();
                }
            }

            break;
        }

        case FILE_FLIP_HORZ: {
            if (pathName[0] != NULL)
            {

                if (checkFileIntegrity(pathName) == 0) {
                    openbmpfile(pathName);
                    flipImage();
                    strcpy(pathName, tmpPath);
                    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

                }
                else
                {
                    noFileSelected();
                }

            }
            else
            {
                noFileSelected();

            }

            break;
        }

        case FILE_FLIP_VERT: {

            if (pathName[0] != NULL) {

                if (checkFileIntegrity(pathName) == 0) {
                    openbmpfile(pathName);
                    flipImageVert(main_header, main_dibheader, main_pic);
                    strcpy(pathName, tmpPath);
                    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                }
                else
                {
                    noFileSelected();
                }
            }
            else
            {
                noFileSelected();
            }

            break;
        }

        case FILE_BW:
            if (pathName[0] != NULL)
            {

                if (checkFileIntegrity(pathName) == 0) {
                    openbmpfile(pathName);
                    RGBtoGrayscale(main_header, main_dibheader, main_pic);
                    strcpy(pathName, tmpPath);
                    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

                }
                else
                {
                    noFileSelected();
                }

            }
            else
            {
                noFileSelected();

            }
            break;

        case FILE_QUIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        }

        break;

    case WM_RBUTTONUP: // When right button released
        point.x = LOWORD(lParam); //Low order for x coords
        point.y = HIWORD(lParam); //High order for y coords

        hMenuPopup = CreatePopupMenu();
        ClientToScreen(hwnd, &point); // Converts the client coordinates to a specified point to screen coordinates to display the context menu


        create_menus(hMenuPopup);

        TrackPopupMenu(hMenuPopup, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL); // Displays the context menu in a given location & tracks the selection of the items in the menu
        DestroyMenu(hMenuPopup);
        break;

    case WM_SIZE:
        SetWindowPos(ghwndEdit, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam),
            SWP_NOMOVE | SWP_NOZORDER);

        break;

    case WM_DESTROY:

        DeleteObject(hBitmap);
        remove(tmpPath);
        PostQuitMessage(0);
        return 0;

    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);

}
