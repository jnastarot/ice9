#pragma once

int WINAPI h_DispatchClientMessage_A_win32(HWND* hwnd, DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc);
int WINAPI h_DispatchClientMessage_W_win32(HWND* hwnd, DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc);

int WINAPI h_DispatchClientMessage_A_win64(HWND* hwnd,
#ifdef _M_IX86
    DWORD aling_32,
#endif	
    DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc);

int WINAPI h_DispatchClientMessage_W_win64(HWND* hwnd,
#ifdef _M_IX86
    DWORD aling_32,
#endif	
    DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc);


void ice9::set_window_hook_exp() {
    void * fnClientA, *fnClientW, *fnClientWorker;
    _RtlRetrieveNtUserPfn RtlRetrieveNtUserPfn = (_RtlRetrieveNtUserPfn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlRetrieveNtUserPfn");
#ifdef ICE9_DEBUG
    printf("RtlRetrieveNtUserPfn[%p] \n", RtlRetrieveNtUserPfn);
#endif
    if (RtlRetrieveNtUserPfn && RtlRetrieveNtUserPfn(&fnClientA, &fnClientW, &fnClientWorker) == STATUS_SUCCESS ) {
#ifdef ICE9_DEBUG
        printf("fnClientA[%p] fnClientW[%p] fnClientWorker[%p]\n", fnClientA, fnClientW, fnClientWorker);
#endif
        if (fnClientA && fnClientW) {
            /*
              pfnClient->
              0x0 pFunc  ----->[ jmp [ntdll_vtbl[1]] ]->_user32_func1
              0x8 pFunc  ----->[ jmp [ntdll_vtbl[2]] ]->_user32_func2
              0x10 pFunc ----->[ jmp [ntdll_vtbl[3]] ]->_user32_func3
              0x18 pFunc ----->[ jmp [ntdll_vtbl[4]] ]->_user32_func4
              ...
              ...
              */

            if (is_64) {
#ifdef _M_IX86
                vtbl_pDispatchA = (void*)*(DWORD*)((DWORD)((pfnClient)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 2);
                vtbl_pDispatchW = (void*)*(DWORD*)((DWORD)((pfnClient)fnClientW)->element[_DispatchClientMessage_idx].pFunc + 2);
#else
                vtbl_pDispatchA = (void*)(*(DWORD*)((DWORD64)((pfnClient)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 2) +
                    (DWORD64)((pfnClient)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 6);
                vtbl_pDispatchW = (void*)(*(DWORD*)((DWORD64)((pfnClient)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 2) +
                    (DWORD64)((pfnClient)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 6);
#endif
            }
            else {
                vtbl_pDispatchA = (void*)*(DWORD*)((DWORD)((pfnClient32)fnClientA)->element[_DispatchClientMessage_idx].pFunc + 2);
                vtbl_pDispatchW = (void*)*(DWORD*)((DWORD)((pfnClient32)fnClientW)->element[_DispatchClientMessage_idx].pFunc + 2);
            }


            o_DispatchClientMessage_A = (BYTE*)*(DWORD*)vtbl_pDispatchA;
            o_DispatchClientMessage_W = (BYTE*)*(DWORD*)vtbl_pDispatchW;
#ifdef ICE9_DEBUG
            printf("vtbl_pDispatchA[%p] vtbl_pDispatchW[%p] \n", vtbl_pDispatchA, vtbl_pDispatchW);
            printf("o_DispatchClientMessage_A[%p] o_DispatchClientMessage_W[%p] \n", o_DispatchClientMessage_A, o_DispatchClientMessage_W);
#endif
            DWORD lastProt_a;
            DWORD lastProt_w;

            VirtualProtect((void*)vtbl_pDispatchA, 0xB8, PAGE_EXECUTE_READWRITE, &lastProt_a);
            VirtualProtect((void*)vtbl_pDispatchW, 0xB8, PAGE_EXECUTE_READWRITE, &lastProt_w);

            if (is_64) {
#ifdef _M_IX86
                *(DWORD*)vtbl_pDispatchA = (DWORD)h_DispatchClientMessage_A_win64;
                *(DWORD*)vtbl_pDispatchW = (DWORD)h_DispatchClientMessage_W_win64;
#else
                *(DWORD64*)vtbl_pDispatchA = (DWORD64)h_DispatchClientMessage_A_win64;
                *(DWORD64*)vtbl_pDispatchW = (DWORD64)h_DispatchClientMessage_W_win64;
#endif
            }
            else {
                *(DWORD*)vtbl_pDispatchA = (DWORD)h_DispatchClientMessage_A_win32;
                *(DWORD*)vtbl_pDispatchW = (DWORD)h_DispatchClientMessage_W_win32;
            }

            VirtualProtect((void*)vtbl_pDispatchA, 0xB8, lastProt_a, &lastProt_a);
            VirtualProtect((void*)vtbl_pDispatchW, 0xB8, lastProt_w, &lastProt_w);
        }
    }
}

void ice9::unset_window_hook_exp() {

    if (vtbl_pDispatchA && vtbl_pDispatchW) {

        DWORD lastProt_a;
        DWORD lastProt_w;

        VirtualProtect((void*)vtbl_pDispatchA, 0xB8, PAGE_EXECUTE_READWRITE, &lastProt_a);
        VirtualProtect((void*)vtbl_pDispatchW, 0xB8, PAGE_EXECUTE_READWRITE, &lastProt_w);

        if (is_64) {
            *(DWORD64*)vtbl_pDispatchA = (DWORD64)o_DispatchClientMessage_A;
            *(DWORD64*)vtbl_pDispatchW = (DWORD64)o_DispatchClientMessage_W;
        }
        else {
            *(DWORD*)vtbl_pDispatchA = (DWORD)o_DispatchClientMessage_A;
            *(DWORD*)vtbl_pDispatchW = (DWORD)o_DispatchClientMessage_W;
        }

        VirtualProtect((void*)vtbl_pDispatchA, 0xB8, lastProt_a, &lastProt_a);
        VirtualProtect((void*)vtbl_pDispatchW, 0xB8, lastProt_w, &lastProt_w);

    }
}


inline bool dispatched_check_msg(DWORD msg) {
    switch (msg) {
        //keyborad
    case WM_KEYDOWN:case WM_KEYUP:
        //mouse
    case WM_RBUTTONDOWN:case WM_LBUTTONDOWN:case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:case WM_RBUTTONUP:case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:{
#ifdef ICE9_DEBUG
        printf("[DispatchClient] Device Emulation [SendMessage]\n");
#endif
        return false;
    }
    default:
        return true;
    }
}



int WINAPI h_DispatchClientMessage_A_win64(HWND* hwnd,
#ifdef _M_IX86
    DWORD aling_32,
#endif	
    DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc) {

    switch (msg) {
        if (!dispatched_check_msg(msg)) {
            ice9().instance().on_event(ice9_message_emulated);
            return 0;
        }
    }

    return  ((_DispatchClientMessagex64)ice9().instance().get_dispatch_client_message_a())(hwnd,
#ifdef _M_IX86
        aling_32,
#endif
        msg, wparam, lparam, proc);
}

int WINAPI h_DispatchClientMessage_W_win64(HWND* hwnd,
#ifdef _M_IX86
    DWORD aling_32,
#endif	
    DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc) {

    if (hwnd) {
        if (!dispatched_check_msg(msg)) {
            ice9().instance().on_event(ice9_message_emulated);
            return 0;
        }
    }

    return ((_DispatchClientMessagex64)ice9().instance().get_dispatch_client_message_w())(hwnd,
#ifdef _M_IX86
        aling_32,
#endif
        msg, wparam, lparam, proc);
}

int WINAPI h_DispatchClientMessage_A_win32(HWND* hwnd, DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc) {

    switch (msg) {
        if (!dispatched_check_msg(msg)) {
            ice9().instance().on_event(ice9_message_emulated);
            return 0;
        }
    }

    return ((_DispatchClientMessagex32)ice9().instance().get_dispatch_client_message_a())(hwnd, msg, wparam, lparam, proc);
}

int WINAPI h_DispatchClientMessage_W_win32(HWND* hwnd, DWORD msg, WPARAM wparam, LPARAM lparam, WNDPROC proc) {

    if (hwnd) {
        if (!dispatched_check_msg(msg)) {
            ice9().instance().on_event(ice9_message_emulated);
            return 0;
        }
    }

    return ((_DispatchClientMessagex32)ice9().instance().get_dispatch_client_message_w())(hwnd, msg, wparam, lparam, proc);
}