#pragma once
LRESULT CALLBACK window_hook_handler(int code, WPARAM wParam, LPARAM lParam);

void ice9::set_window_hook(msg_manager& mgr) {
    mgr.hook = SetWindowsHookExA(WH_GETMESSAGE, window_hook_handler,0,GetWindowThreadProcessId(mgr.hwnd,0));
}
void ice9::unset_window_hook(msg_manager& mgr) {
    UnhookWindowsHookEx(mgr.hook);
}


LRESULT CALLBACK window_hook_handler(int code, WPARAM wParam, LPARAM lParam) {

    MSG * message = (MSG*)lParam;

    for (auto& win_mgr : ice9().instance().get_windows()) {
        if (win_mgr.hwnd == message->hwnd) {

            switch (message->message) {
            case WM_LBUTTONDOWN: {
                win_mgr.mouse_press[0] = true;
                break;
            }
            case WM_LBUTTONUP: {
                win_mgr.mouse_press[0] = false;
                break;
            }
            case WM_MBUTTONDOWN: {
                win_mgr.mouse_press[1] = true;
                break;
            }
            case WM_MBUTTONUP: {
                win_mgr.mouse_press[1] = false;
                break;
            }
            case WM_RBUTTONDOWN: {
                win_mgr.mouse_press[2] = true;
                break;
            }
            case WM_RBUTTONUP: {
                win_mgr.mouse_press[2] = false;
                break;
            }

            default:break;
            }

            return CallNextHookEx(0, code, wParam, lParam);
        }
    }
 
    return 1;
}