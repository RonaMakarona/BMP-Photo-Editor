

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
    AppendMenuW(hMenuFile, MF_STRING, FILE_COLOR, L"&Color Picker");
    AppendMenuW(hMenuFile, MF_STRING, FILE_SAVE, L"&Save");

    AppendMenuW(hMenuFile, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenuFile, L"&Flip");
    AppendMenuW(hSubMenuFile, MF_STRING, FILE_FLIP_HORZ, L"&Flip Horizontally");
    AppendMenuW(hSubMenuFile, MF_STRING, FILE_FLIP_VERT, L"&Flip Vertically");

    AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenuFile, MF_STRING, FILE_QUIT, L"&Quit");

    AppendMenuW(hMenuHelp, MF_STRING, FILE_NEW, L"&Test");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuFile, L"&File");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuHelp, L"&Test");
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
    ofn.lpstrFilter = TEXT("bmp files (*.BMP)\0*.bmp\0");
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
    // IDM_NEW, IDM_OPEN, and IDM_SAVE are application-defined command constants.

    TBBUTTON tbButtons[numToolbarButtons] =
    {
        { MAKELONG(STD_FILENEW,  ImageListID), FILE_NEW,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("New")},
        { MAKELONG(STD_FILEOPEN, ImageListID), FILE_OPEN, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Open")},
        { MAKELONG(STD_FILESAVE, ImageListID), FILE_SAVE, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("Save")},
        { MAKELONG(STD_PROPERTIES, ImageListID), FILE_BW, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)TEXT("B&&W")}
    };

    // Add buttons.
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numToolbarButtons, (LPARAM)&tbButtons);

    // Resize the toolbar, and then show it.
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(hWndToolbar, TRUE);

    return hWndToolbar;
}
