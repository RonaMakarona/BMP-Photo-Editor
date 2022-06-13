#include <WinUser.h>


void AddMenus(HWND hwnd) {

    HMENU hMenubar;

    HMENU hMenuFile;
    HMENU hSubMenuFile;

    HMENU hMenuHelp;


    hMenubar = CreateMenu();

    hMenuFile = CreateMenu();
    hSubMenuFile = CreateMenu();

    hMenuHelp = CreateMenu();

    AppendMenuW(hMenuFile, MF_STRING, FILE_NEW, L"&New");
    AppendMenuW(hMenuFile, MF_STRING, FILE_OPEN, L"&Open");
    AppendMenuW(hMenuFile, MF_STRING, FILE_SAVE, L"&Save");

    AppendMenuW(hMenuFile, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenuFile, L"&Flip");
    AppendMenuW(hSubMenuFile, MF_STRING, FILE_FLIP_HORZ, L"&Flip Horizontally");
    AppendMenuW(hSubMenuFile, MF_STRING, FILE_FLIP_VERT, L"&Flip Vertically");

    AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenuFile, MF_STRING, FILE_QUIT, L"&Quit");

    AppendMenuW(hMenuHelp, MF_STRING, FILE_HOWTO, L"&How to use"); //TODO: Add tips like in ghidra
    AppendMenuW(hMenuHelp, MF_STRING, FILE_ABOUT, L"&About"); 

    // Main menubar options
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuFile, L"&File"); 
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuHelp, L"&Help");
    SetMenu(hwnd, hMenubar);

}

void OpenDialog(HWND hwnd) {

    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.hwndOwner = hwnd;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("bmp files (*.BMP)\0*.bmp\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        strcpy(pathName, szFile);
        if (checkFileIntegrity(pathName) == 1) {
            invalidFile(hwnd);
            return;
        }
        openbmpfile(pathName);
        strcpy(pathName, tmpPath); // check if needed
        writeImageToSave(tmpPath);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);


    }
}

void SaveDialog(HWND hwnd) {

    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.hwndOwner = hwnd;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("bmp files (*.BMP)\0*.bmp\0"); // file filter
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrDefExt = "bmp";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        strcpy(pathName, szFile);
        writeImageToSave(pathName);
    }
}

HWND CreateSimpleToolbar(HWND hWndParent)
{
    // Declare and initialize local constants.
    const int ImageListID = 0;

    const DWORD buttonStyles = BTNS_AUTOSIZE;

    // Create the toolbar.
    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | TBSTYLE_WRAPABLE, 0, 0, 0, 0,
        hWndParent, NULL, NULL, NULL);

    if (hWndToolbar == NULL)
        return NULL;

    // Create the image list.
    g_hImageList = ImageList_Create(ToolbarBitmapSize, ToolbarBitmapSize,   // Dimensions of individual bitmaps.
        ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
        numToolbarButtons, 0);

    // Set the image list.
    SendMessage(hWndToolbar, TB_SETIMAGELIST,
        (WPARAM)ImageListID,
        (LPARAM)g_hImageList);

    // Load the button images.
    SendMessage(hWndToolbar, TB_LOADIMAGES,
        (WPARAM)IDB_STD_SMALL_COLOR,
        (LPARAM)HINST_COMMCTRL);

    // Initialize button info.

    TBBUTTON tbButtons[numToolbarButtons] =
    {
        { MAKELONG(STD_FILENEW,  ImageListID), FILE_NEW,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("New")},
        { MAKELONG(STD_FILEOPEN, ImageListID), FILE_OPEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Open")},
        { MAKELONG(STD_FILESAVE, ImageListID), FILE_SAVE, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Save")},
        { MAKELONG(STD_PROPERTIES, ImageListID), FILE_BW, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("B&&W")},
        { MAKELONG(STD_HELP, ImageListID), FILE_BRIGHTEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Brighten")},
        { MAKELONG(STD_REPLACE, ImageListID), FILE_CONTRAST, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Contrast RGB")}
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numToolbarButtons, (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}

void CreateControlsBrightness(HWND hwnd) {

    // Creates the trackbar controls

    HWND hLeftLabel = CreateWindowW(L"Static", L"-50",
        WS_CHILD | WS_VISIBLE, 0, 0, 30, 30, hwnd, (HMENU)1, NULL, NULL);

    HWND hRightLabel = CreateWindowW(L"Static", L"50",
        WS_CHILD | WS_VISIBLE, 0, 0, 30, 30, hwnd, (HMENU)2, NULL, NULL);

    //Will display the number currently selected
    hlbl = CreateWindowW(L"Static", L"0", WS_CHILD | WS_VISIBLE,
        270, 20, 30, 30, hwnd, (HMENU)3, NULL, NULL);
    
    HWND hButton = CreateWindow(TEXT("button"), TEXT("Enter"), 
       WS_VISIBLE | WS_CHILD, 135, 70, 50, 30, hwnd, (HMENU)4, NULL, NULL);

    INITCOMMONCONTROLSEX icex;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    hTrack = CreateWindowW(TRACKBAR_CLASSW, L"Trackbar Control", //Using the Trackbar class
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, //Creates tick marks
        40, 20, 170, 30, hwnd, (HMENU)3, NULL, NULL);

    SendMessageW(hTrack, TBM_SETRANGE, TRUE, MAKELONG(-50, 50)); //trackbar range
    SendMessageW(hTrack, TBM_SETPAGESIZE, 0, 10); // page size
    SendMessageW(hTrack, TBM_SETTICFREQ, 10, 0); //tick frequency
    SendMessageW(hTrack, TBM_SETPOS, FALSE, 0); //slider position
    SendMessageW(hTrack, TBM_SETBUDDY, TRUE, (LPARAM)hLeftLabel); 
    SendMessageW(hTrack, TBM_SETBUDDY, FALSE, (LPARAM)hRightLabel);
}


void UpdateLabelBrightness(void) {

    LRESULT pos = SendMessageW(hTrack, TBM_GETPOS, 0, 0); // getting the current position of the trackbar
    wchar_t buf[4];
    wsprintfW(buf, L"%ld", pos);

    brightNum = pos;
    SetWindowTextW(hlbl, buf); // setting the new position as text
}


