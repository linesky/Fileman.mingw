#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define IDC_LISTVIEW 101
#define IDC_BUTTON_CREATE_FOLDER 102
//gcc main.c -o directory_list_view -lcomctl32 -lshell32 -lshlwapi
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitListViewColumns(HWND hWndListView);
void InsertListViewItems(HWND hWndListView);
void OpenSelectedFile(HWND hWndListView);
void CreateNewFolder(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "DirectoryListViewClass";
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Directory List View",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hWndListView;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        //InitCommonControlsEx icex;
        //icex.dwICC = ICC_LISTVIEW_CLASSES;
        //InitCommonControlsEx(&icex);

        hWndListView = CreateWindow(WC_LISTVIEW, "",
            WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ICON | LVS_EDITLABELS,
            10, 10, 560, 300,
            hwnd, (HMENU)IDC_LISTVIEW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        InitListViewColumns(hWndListView);
        InsertListViewItems(hWndListView);

        HWND hWndButton = CreateWindow(
            "BUTTON",
            "Create New Folder",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 320, 200, 30,
            hwnd, (HMENU)IDC_BUTTON_CREATE_FOLDER, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        break;
    }
    case WM_NOTIFY:
    {
        if (((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW && ((LPNMHDR)lParam)->code == NM_DBLCLK)
        {
            OpenSelectedFile(hWndListView);
        }
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_CREATE_FOLDER)
        {
            CreateNewFolder(hwnd);
            InsertListViewItems(hWndListView); // Refresh the list view
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void InitListViewColumns(HWND hWndListView)
{
    LVCOLUMN lvColumn;

    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.cx = 150;
    lvColumn.pszText = "Name";
    lvColumn.iSubItem = 0;
    ListView_InsertColumn(hWndListView, 0, &lvColumn);

    lvColumn.pszText = "Type";
    lvColumn.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &lvColumn);

    lvColumn.pszText = "Size";
    lvColumn.iSubItem = 2;
    ListView_InsertColumn(hWndListView, 2, &lvColumn);
}

void InsertListViewItems(HWND hWndListView)
{
    ListView_DeleteAllItems(hWndListView);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Invalid file handle. Error: %u\n", "Error", MB_OK);
        return;
    }
    else
    {
        do
        {
            LVITEM lvItem;
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lvItem.iItem = ListView_GetItemCount(hWndListView);
            lvItem.iSubItem = 0;
            lvItem.pszText = findFileData.cFileName;
            lvItem.iImage = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : 1;

            ListView_InsertItem(hWndListView, &lvItem);

            ListView_SetItemText(hWndListView, lvItem.iItem, 1, (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "Folder" : "File");

            char size[20];
            sprintf(size, "%llu", ((unsigned long long)findFileData.nFileSizeHigh << 32) + findFileData.nFileSizeLow);
            ListView_SetItemText(hWndListView, lvItem.iItem, 2, size);

        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    }
}

void OpenSelectedFile(HWND hWndListView)
{
    int iSelected = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);

    if (iSelected != -1)
    {
        char filePath[MAX_PATH];
        ListView_GetItemText(hWndListView, iSelected, 0, filePath, MAX_PATH);

        ShellExecute(NULL, "open", filePath, NULL, NULL, SW_SHOWNORMAL);
    }
}

void CreateNewFolder(HWND hwnd)
{
    char newFolderName[MAX_PATH] = "NewFolder";
    int folderNumber = 1;

    while (PathFileExists(newFolderName))
    {
        sprintf(newFolderName, "NewFolder%d", folderNumber++);
    }

    if (!CreateDirectory(newFolderName, NULL))
    {
        MessageBox(hwnd, "Could not create new folder.", "Error", MB_OK | MB_ICONERROR);
    }
}

