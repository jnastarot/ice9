#pragma once
#pragma comment(linker, "/EXPORT:CreateWindowExA=_imp_CreateWindowExA@48")
#pragma comment(linker, "/EXPORT:CreateWindowExW=_imp_CreateWindowExW@48")
#pragma comment(linker, "/EXPORT:DestroyWindow=_imp_DestroyWindow@4")

LRESULT CALLBACK raw_proc_filter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#include "ice9_window_hook.h"
#include "ice9_window_hook_experimental.h"
#include "ice9_window_raw_hook.h"

typedef HWND (WINAPI* j_CreateWindowExA)(DWORD dwExStyle,LPCSTR lpClassName,LPCSTR lpWindowName,
    DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);

typedef HWND(WINAPI* j_CreateWindowExW)(DWORD dwExStyle,LPCWSTR lpClassName,LPCWSTR lpWindowName,
    DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);

typedef BOOL(WINAPI* j_DestroyWindow)(HWND hWnd);

DLLEXPORT HWND WINAPI imp_CreateWindowExA(
    _In_ DWORD dwExStyle,
    _In_opt_ LPCSTR lpClassName,
    _In_opt_ LPCSTR lpWindowName,
    _In_ DWORD dwStyle,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID lpParam) {

    HWND ret_hwnd = j_CreateWindowExA(GetProcAddress(GetModuleHandleA("user32.dll"), "CreateWindowExA"))(
        dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam
        );

    if (ret_hwnd) {
        ice9().instance().on_window_attach(ret_hwnd);
    }

    return ret_hwnd;
}

DLLEXPORT HWND WINAPI imp_CreateWindowExW(
    _In_ DWORD dwExStyle,
    _In_opt_ LPCWSTR lpClassName,
    _In_opt_ LPCWSTR lpWindowName,
    _In_ DWORD dwStyle,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu,
    _In_opt_ HINSTANCE hInstance,
    _In_opt_ LPVOID lpParam) {

    HWND ret_hwnd = j_CreateWindowExW(GetProcAddress(GetModuleHandleA("user32.dll"), "CreateWindowExW"))(
        dwExStyle, lpClassName, lpWindowName, dwStyle,X,Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam
        );

    if (ret_hwnd) {
        ice9().instance().on_window_attach(ret_hwnd);
    } 

    return ret_hwnd;
}


DLLEXPORT BOOL WINAPI imp_DestroyWindow(HWND hWnd) {

    BOOL ret = j_DestroyWindow(GetProcAddress(GetModuleHandleA("user32.dll"), "DestroyWindow"))(hWnd);

    if (ret) {
        ice9().instance().on_window_detach(hWnd);
    }

    return ret;
}

void ice9::on_window_attach(HWND hwnd) {
#ifdef ICE9_DEBUG
    printf("window attached [%x]\n", hwnd);
#endif
    msg_manager win_mgr;
    ZeroMemory(&win_mgr, sizeof(msg_manager));
    win_mgr.hwnd = hwnd;
    
#ifdef ICE9_WINDOW_HOOK_ENABLED
    set_window_hook(win_mgr);
#endif 
#ifdef ICE9_WINDOW_RAW_HOOK_ENABLED
    set_raw_window_hook(win_mgr);
#endif
    windows.push_back(win_mgr);
}

void ice9::on_window_detach(HWND hwnd) {
#ifdef ICE9_DEBUG
    printf("window detached [%x]\n", hwnd);
#endif

    for (unsigned int item_mgr_idx = 0; item_mgr_idx < windows.size(); item_mgr_idx++) {
        if (windows[item_mgr_idx].hwnd == hwnd) {
#ifdef ICE9_WINDOW_HOOK_ENABLED
            unset_window_hook(windows[item_mgr_idx]); 
#endif
#ifdef ICE9_WINDOW_RAW_HOOK_ENABLED
            unset_raw_window_hook(windows[item_mgr_idx]);
#endif
            windows.erase(windows.begin() + item_mgr_idx);
        }
    }
}

LRESULT CALLBACK raw_proc_filter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    for (auto& win_mgr : ice9().instance().get_windows()) {
        if (win_mgr.hwnd == hWnd) {

           switch (message) {
            case WM_LBUTTONDOWN: {
                if (!win_mgr.mouse_press[0]) {
                    ice9().instance().on_event(ice9_message_emulated);
#ifdef ICE9_DEBUG
                    printf("Device Emulation [SendMessage]\n");
#endif
                    return 0;
                }
                break;
            }
            case WM_MBUTTONDOWN: {
                if (!win_mgr.mouse_press[1]) {
                    ice9().instance().on_event(ice9_message_emulated);
#ifdef ICE9_DEBUG
                    printf("Device Emulation [SendMessage]\n");
#endif
                    return 0;
                }
                break;
            }
            case WM_RBUTTONDOWN: {
                if (!win_mgr.mouse_press[2]) {
                    ice9().instance().on_event(ice9_message_emulated);
#ifdef ICE9_DEBUG
                    printf("Device Emulation [SendMessage]\n");
#endif
                    return 0;
                }
                break;
            }

            //KeyBoard
            case WM_KEYDOWN: {
                if (!win_mgr.key_press[wParam].down) {   
                    ice9().instance().on_event(ice9_message_emulated);
#ifdef ICE9_DEBUG
                    printf("Device Emulation [SendMessage]\n");
#endif
                    return 0;
                }
                else {
                    win_mgr.key_press[wParam].down--;
                }
                break;
            }
            case WM_KEYUP: {
                if (!win_mgr.key_press[wParam].up) {
                    ice9().instance().on_event(ice9_message_emulated);
#ifdef ICE9_DEBUG
                    printf("Device Emulation [SendMessage]\n");
#endif
                    return 0;
                }
                else {
                    win_mgr.key_press[wParam].up--;
                }
                break;
            }


            case WM_INPUT: {


                if (GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT) {
                    BYTE *lpb = NULL;
                    UINT dwSize = sizeof(RAWINPUT);

                    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                    lpb = new BYTE[dwSize];


                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
                        return 0;
                    }

                    if (((RAWINPUT*)(lpb))->header.hDevice) {
                        if (((RAWINPUT*)(lpb))->data.keyboard.Message == WM_KEYDOWN) {
                            win_mgr.key_press[(BYTE)((RAWINPUT*)(lpb))->data.keyboard.VKey].down++;
                        }
                        if (((RAWINPUT*)(lpb))->data.keyboard.Message == WM_KEYUP) {
                            win_mgr.key_press[(BYTE)((RAWINPUT*)(lpb))->data.keyboard.VKey].up++;
                        }
                        delete[] lpb;
                    }
                    else {
                        ice9().instance().on_event(ice9_input_emulated);
#ifdef ICE9_DEBUG
                        printf("Device Emulation [SendInput]\n");
#endif
                        delete[] lpb;
                        return 0;
                    }
                }
                break;
            }

            default: break;
            }

            return win_mgr.proc(hWnd, message, wParam, lParam);
        }
    }

#ifdef ICE9_DEBUG
    printf("window not found hwnd[%s] msg[%x]\n", hWnd, message);
#endif

    ice9().instance().on_event(ice9_window_not_found);
    return 0;
}