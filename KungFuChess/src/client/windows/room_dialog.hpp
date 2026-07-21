#pragma once
#include <windows.h>
#include <string>
#include "dialog_results.hpp"

class RoomDialog {
private:
    inline static RoomResult* s_result = nullptr;
    inline static HWND hRoom = nullptr;

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 3) {
                DestroyWindow(hwnd); 
                return 0;
            }
            if (LOWORD(wParam) == 1 || LOWORD(wParam) == 2) {
                char room[256];
                GetWindowTextA(hRoom, room, 256);

                if (s_result) {
                    s_result->roomName = room;
                    s_result->action = (LOWORD(wParam) == 1) ? "CREATE_ROOM" : "JOIN_ROOM";
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
    static RoomResult ShowDialog() {
        RoomResult res;
        s_result = &res;

        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSA wc = { 0 };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "RoomDialogClass";
        RegisterClassA(&wc);

        HWND hwnd = CreateWindowExA(0, "RoomDialogClass", "Room Selection",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 200,
            NULL, NULL, hInstance, NULL);

        CreateWindowA("STATIC", "Room Name:", WS_VISIBLE | WS_CHILD, 20, 30, 80, 20, hwnd, NULL, hInstance, NULL);
        hRoom = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 110, 30, 190, 20, hwnd, NULL, hInstance, NULL);

        CreateWindowA("BUTTON", "Create", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 80, 90, 35, hwnd, (HMENU)1, hInstance, NULL);
        CreateWindowA("BUTTON", "Join", WS_VISIBLE | WS_CHILD, 120, 80, 90, 35, hwnd, (HMENU)2, hInstance, NULL);
        CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD, 220, 80, 90, 35, hwnd, (HMENU)3, hInstance, NULL);

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
        UnregisterClassA("RoomDialogClass", hInstance);
        
        return res;
    }
};