#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <mshtmcid.h>
#include <tchar.h>
#include <WinUser.h>


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
#define FILE_BRIGHTEN 8
#define FILE_QUIT 9

#define IDI_SMALL 108 // for icon

#define numToolbarButtons 5 //num of toolbar buttons

#define create_menus(menu)\
AppendMenuW(menu, MF_STRING, FILE_NEW, L"&New");\
AppendMenuW(menu, MF_STRING, FILE_OPEN, L"&Open");\
AppendMenuW(menu, MF_STRING, FILE_COLOR, L"&Color Picker");\
AppendMenuW(menu, MF_SEPARATOR, 0, NULL);\
AppendMenuW(menu, MF_STRING, FILE_QUIT, L"&Quit");\

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcPopup(HWND, UINT, WPARAM, LPARAM);

ATOM RegisterBrightnessClass(HINSTANCE hInstance);

void AddMenus(HWND);
void OpenDialog(HWND);
void SaveDialog(HWND);
HWND CreateSimpleToolbar(HWND);
void CreateControls(HWND hwnd);
void UpdateLabel(void);

HWND ghwndEdit;

// main window HWND
HWND mainHWND;

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

// trackbar variables
HWND hTrack;
HWND hlbl;

// popup window
HWND popupHWND;

// for brighten func
int brightNum = 0;


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
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    //wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassW(&wc);

    RegisterBrightnessClass(hInstance);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Very cool editor",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 720, 480, NULL, NULL, hInstance, NULL);
    mainHWND = hwnd;



    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }

    return (int)msg.wParam;
}

ATOM RegisterBrightnessClass(HINSTANCE hInstance)
{
    WNDCLASSW wcp = { 0 };

    wcp.style = CS_HREDRAW | CS_VREDRAW;
    wcp.lpfnWndProc = WndProcPopup;
    wcp.cbClsExtra = 0;
    wcp.cbWndExtra = 0;
    wcp.hInstance = hInstance;
    wcp.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcp.lpszClassName = L"Brightness";

    return RegisterClassW(&wcp);
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

        // Creates in hidden mode the brightness trackbar
        popupHWND = CreateWindowW(L"Brightness", L"Brightness Trackbar",
            (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) & ~WS_MAXIMIZEBOX | WS_BORDER | WS_EX_TOPMOST, 850, 200, 320, 150, NULL, NULL, NULL, NULL);

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

        //SetStretchBltMode(hdc, HALFTONE);
        //GetClientRect(hwnd, &rc);
        //bResult = StretchBlt(hdc, 5, 47, rc.right, rc.bottom,
        //    hdcMem, 0, 0, bitmap.bmWidth - 5, bitmap.bmHeight - 47, SRCCOPY);

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

            if (*pathName != NULL) {

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


        case FILE_BRIGHTEN:
            if (pathName[0] != NULL)
            {

                if (checkFileIntegrity(pathName) == 0) {
                    openbmpfile(pathName);


                    //HWND popupHWND = CreateWindow(L"Settings", L"Settings", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, NULL, NULL);
                    //ShowWindow(popupHWND, SW_SHOW);
                    //UpdateWindow(popupHWND);

                    ShowWindow(popupHWND, SW_SHOW);

                    //TODO: open slidebar, create popup window
                  

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

    case WM_DROPFILES: {
        MessageBox(hwnd, "Dragged!", "Title", MB_OK | MB_ICONINFORMATION);
        break;
    }

    case WM_SIZE:
        SetWindowPos(ghwndEdit, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam),
            SWP_NOMOVE | SWP_NOZORDER);

        break;


    case WM_HSCROLL: //Gets called when we move the slider of the trackbar
        UpdateLabel();
        break;


    case WM_DESTROY:

        DeleteObject(hBitmap);
        remove(tmpPath);
        PostQuitMessage(0);
        return 0;

    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);

}


// Popup
LRESULT CALLBACK WndProcPopup(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {

    switch (msg) {

    case WM_CREATE:
       CreateControls(hwnd);
       break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 4) {
            brightenRGB(main_header, main_dibheader, main_pic, brightNum);
            strcpy(pathName, tmpPath);
            RedrawWindow(mainHWND, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
    case WM_HSCROLL:
        UpdateLabel();
        break;

    case WM_CLOSE:
        ShowWindow(popupHWND, SW_HIDE);
        break;

    case WM_DESTROY:
        
       break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

  
}
