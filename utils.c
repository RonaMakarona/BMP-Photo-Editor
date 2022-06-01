void invalidFile(HWND hwnd) {
    MessageBoxW(NULL, L"File invalid", L"Error", MB_OK);
    strcpy(pathName, tmpPath);
}

void noFileSelected() {
    MessageBoxW(NULL, L"No file selected", L"Error", MB_OK);
}

void couldntOpen() {
    MessageBoxW(NULL, L"Could not open file", L"Error", MB_OK);
}