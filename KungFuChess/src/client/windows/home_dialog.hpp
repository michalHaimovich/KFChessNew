#pragma once
#include <windows.h>
#include "dialog_results.hpp"

class HomeDialog {
private:
    inline static HomeResult* s_result = nullptr;

   static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_COMMAND:
            if (s_result) {
                if (LOWORD(wParam) == 1) s_result->action = HomeAction::PLAY_RANDOM;
                else if (LOWORD(wParam) == 2) s_result->action = HomeAction::ENTER_ROOM;
            }
            DestroyWindow(hwnd); 
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }

public:
    static HomeResult ShowDialog(const std::string& username) {
        HomeResult res;
        s_result = &res;

        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSA wc = { 0 };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "HomeDialogClass";
        RegisterClassA(&wc);

        std::string title = "Welcome, " + username;
        HWND hwnd = CreateWindowExA(0, "HomeDialogClass", title.c_str(),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 200,
            NULL, NULL, hInstance, NULL);

        CreateWindowA("STATIC", "Choose your game mode:", WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 20, 300, 20, hwnd, NULL, hInstance, NULL);

        CreateWindowA("BUTTON", "Play (Random Match)", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 25, 60, 140, 45, hwnd, (HMENU)1, hInstance, NULL);
        CreateWindowA("BUTTON", "Room", WS_VISIBLE | WS_CHILD, 175, 60, 140, 45, hwnd, (HMENU)2, hInstance, NULL);

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
        UnregisterClassA("HomeDialogClass", hInstance);
        
        return res;
    }
};