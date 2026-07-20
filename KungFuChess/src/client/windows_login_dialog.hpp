#pragma once
#include <windows.h>
#include <string>

struct LoginResult {
    std::string username;
    std::string password;
    std::string roomName;
    std::string action; // "CREATE_ROOM" or "JOIN_ROOM"
    bool success = false;
};

class WindowsLoginDialog {
private:
    static LoginResult* s_result;
    static HWND hUser, hPass, hRoom;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1 || LOWORD(wParam) == 2) {
                char user[256], pass[256], room[256];
                GetWindowTextA(hUser, user, 256);
                GetWindowTextA(hPass, pass, 256);
                GetWindowTextA(hRoom, room, 256);

                if (s_result) {
                    s_result->username = user;
                    s_result->password = pass;
                    s_result->roomName = room;
                    s_result->action = (LOWORD(wParam) == 1) ? "JOIN_ROOM" : "CREATE_ROOM";
                    s_result->success = true;
                }
                PostQuitMessage(0);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }

public:
    static LoginResult ShowDialog() {
        LoginResult res;
        s_result = &res;

        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSA wc = { 0 };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "LoginDialogClass";
        RegisterClassA(&wc);

        HWND hwnd = CreateWindowExA(0, "LoginDialogClass", "Kung Fu Chess - Launcher",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 280,
            NULL, NULL, hInstance, NULL);

        // Labels
        CreateWindowA("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, hInstance, NULL);
        CreateWindowA("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 100, 20, hwnd, NULL, hInstance, NULL);
        CreateWindowA("STATIC", "Room Name:", WS_VISIBLE | WS_CHILD, 20, 100, 100, 20, hwnd, NULL, hInstance, NULL);

        // Edit Boxes
        hUser = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 20, 180, 20, hwnd, NULL, hInstance, NULL);
        hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 120, 60, 180, 20, hwnd, NULL, hInstance, NULL);
        hRoom = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 100, 180, 20, hwnd, NULL, hInstance, NULL);

        // Buttons
        CreateWindowA("BUTTON", "Join Room", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 30, 160, 120, 35, hwnd, (HMENU)1, hInstance, NULL);
        CreateWindowA("BUTTON", "Create Room", WS_VISIBLE | WS_CHILD, 180, 160, 120, 35, hwnd, (HMENU)2, hInstance, NULL);

        // Center window on screen
        RECT rc; GetWindowRect(hwnd, &rc);
        int xPos = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
        int yPos = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
        SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        DestroyWindow(hwnd);
        UnregisterClassA("LoginDialogClass", hInstance);
        
        return res;
    }
};

LoginResult* WindowsLoginDialog::s_result = nullptr;
HWND WindowsLoginDialog::hUser = nullptr;
HWND WindowsLoginDialog::hPass = nullptr;
HWND WindowsLoginDialog::hRoom = nullptr;