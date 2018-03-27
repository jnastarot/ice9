#pragma once

void ice9::set_raw_window_hook(msg_manager& mgr) {

    if (IsWindowUnicode(mgr.hwnd)) {
        mgr.proc = (WNDPROC)SetWindowLongPtrW(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)raw_proc_filter);
    }
    else {
        mgr.proc = (WNDPROC)SetWindowLongPtrA(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)raw_proc_filter);
    }

    if (mgr.proc) {
        RAWINPUTDEVICE rid;
        rid.usUsagePage = 0x01;
        rid.usUsage     = 0x06;
        rid.dwFlags = RIDEV_INPUTSINK;
        rid.hwndTarget = mgr.hwnd;

        if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
            if (IsWindowUnicode(mgr.hwnd)) {
                SetWindowLongPtrW(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)mgr.proc);
            }
            else {
                SetWindowLongPtrA(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)mgr.proc);
            }
        }
    }
}

void ice9::unset_raw_window_hook(msg_manager& mgr) {
    if (IsWindowUnicode(mgr.hwnd)) {
        SetWindowLongPtrW(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)mgr.proc);
    }
    else {
        SetWindowLongPtrA(mgr.hwnd, GWLP_WNDPROC, (LONG_PTR)mgr.proc);
    }
}


