#pragma once
#include <windows.h>
#include <string>
#include "dialog_results.hpp"

class LoginDialog {
private:
    inline static LoginResult* s_result = nullptr;
    inline static HWND hUser = nullptr;
    inline static HWND hPass = nullptr;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { 
                char user[256], pass[256];
                GetWindowTextA(hUser, user, 256);
                GetWindowTextA(hPass, pass, 256);

                if (s_result) {
                    s_result->username = user;
                    s_result->password = pass;
                    s_result->success = true;
                }
                DestroyWindow(hwnd);
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

        HWND hwnd = CreateWindowExA(0, "LoginDialogClass", "Kung Fu Chess - Login",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 320, 200,
            NULL, NULL, hInstance, NULL);

        CreateWindowA("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, hInstance, NULL);
        CreateWindowA("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 20, hwnd, NULL, hInstance, NULL);

        hUser = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 20, 180, 20, hwnd, NULL, hInstance, NULL);
        hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 100, 60, 180, 20, hwnd, NULL, hInstance, NULL);

        CreateWindowA("BUTTON", "Login", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 100, 100, 100, 35, hwnd, (HMENU)1, hInstance, NULL);

        RECT rc; GetWindowRect(hwnd, &rc);
        int xPos = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
        int yPos = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
        SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        
        HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                  DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lParam) -> BOOL {
            SendMessageA(hChild, WM_SETFONT, lParam, TRUE);
            return TRUE;
        }, (LPARAM)hFont);
        
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